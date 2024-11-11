#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "LampeTorche.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AHighlightableObject::AHighlightableObject()
{
    PrimaryActorTick.bCanEverTick = true;

    // Cr�e le composant de maillage uniquement si aucun TargetActor n'est assign�
    if (!TargetActor)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
        RootComponent = MeshComponent;
    }
    isDisplay = false;
}

bool AHighlightableObject::IsIlluminatedByFlashlight(FColor TargetColor)
{
    TArray<AActor*> FoundFlashlights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALampeTorche::StaticClass(), FoundFlashlights);

    for (AActor* FlashlightActor : FoundFlashlights)
    {
        ALampeTorche* Flashlight = Cast<ALampeTorche>(FlashlightActor);

        if (TargetActor)
        {
            return Flashlight->IsActorInDetectionCone(TargetActor, RequiredColor);
        }
        else
        {
            return Flashlight->IsActorInDetectionCone(this, RequiredColor);
        }
    }

    return false;
}


bool AHighlightableObject::getDisplayStatus()
{
    return isDisplay;
}


void AHighlightableObject::HandleObjectStatus()
{
    ALampeTorche* Flashlight = nullptr;
    for (TActorIterator<ALampeTorche> It(GetWorld()); It; ++It)
    {
        Flashlight = *It;
        break;
    }

    if (Flashlight && IsIlluminatedByFlashlight(RequiredColor))
    {
        DisplayObject();
    }
    else
    {
        HideObject();
    }
}

void AHighlightableObject::DisplayObject()
{
    if (TargetActor)
    {
        if (!CheckCollisionWithPlayer(TargetActor->FindComponentByClass<UStaticMeshComponent>()))
        {
            TargetActor->SetActorHiddenInGame(false);
            TargetActor->SetActorEnableCollision(true);
        }
    }
    else
    {
        if (!CheckCollisionWithPlayer(MeshComponent))
        {
            MeshComponent->SetVisibility(true);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }

    isDisplay = true;
}


bool AHighlightableObject::CheckCollisionWithPlayer(UStaticMeshComponent* MeshComponentToTest)
{
    if (!MeshComponentToTest)
    {
        return false;
    }

    UStaticMesh* StaticMesh = MeshComponentToTest->GetStaticMesh();
    if (!StaticMesh)
    {
        return false;
    }
    TArray<FVector> Vertices;
    for (int32 LODIndex = 0; LODIndex < StaticMesh->GetRenderData()->LODResources.Num(); ++LODIndex)
    {
        const FStaticMeshLODResources& LODResources = StaticMesh->GetRenderData()->LODResources[LODIndex];
        const FPositionVertexBuffer& VertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;

        for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); ++i)
        {
            FVector VertexPos = FVector(VertexBuffer.VertexPosition(i).X, VertexBuffer.VertexPosition(i).Y, VertexBuffer.VertexPosition(i).Z);
            FVector WorldVertexPos = MeshComponentToTest->GetComponentTransform().TransformPosition(VertexPos);

            Vertices.Add(WorldVertexPos);
        }
    }
    AActor* PlayerActor = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (PlayerActor)
    {
        FVector PlayerLocation = PlayerActor->GetActorLocation();
        for (const FVector& Vertex : Vertices)
        {
            float DistanceToPlayer = FVector::Dist(Vertex, PlayerLocation);

            if (DistanceToPlayer <= 120.f)
            {
                return true;
            }
        }
    }

    return false;
}


void AHighlightableObject::HideObject()
{
    if (TargetActor)
    {
        TargetActor->SetActorHiddenInGame(true);
        TargetActor->SetActorEnableCollision(false);
        TargetActor->SetHidden(true);
    }
    else
    {
        MeshComponent->SetVisibility(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    isDisplay = false;
}

void AHighlightableObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    HandleObjectStatus();
}

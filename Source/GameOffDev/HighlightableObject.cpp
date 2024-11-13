#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "LampeTorche.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProceduralMeshComponent.h" 
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AHighlightableObject::AHighlightableObject()
{
    PrimaryActorTick.bCanEverTick = true;

    // Crée le composant de maillage uniquement si aucun TargetActor n'est assigné
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
    bool isIlluminated = false;
    for (AActor* FlashlightActor : FoundFlashlights)
    {
        ALampeTorche* Flashlight = Cast<ALampeTorche>(FlashlightActor);
        if (TargetActor)
        {
            isIlluminated = Flashlight->IsActorInDetectionCone(this, TargetActor, RequiredColor);
        }
        else
        {
            isIlluminated = Flashlight->IsActorInDetectionCone(this, this, RequiredColor);
        }
        if (isIlluminated)
            return true;
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


void AHighlightableObject::SetNewMesh(UProceduralMeshComponent* NewMeshToSet)
{
    if (NewMeshToSet)
    {
        if (MeshComponent)
        {
            // Recherche du premier ProceduralMeshComponent attaché à MeshComponent
            for (UActorComponent* Component : MeshComponent->GetAttachChildren())
            {
                UProceduralMeshComponent* ProceduralMesh = Cast<UProceduralMeshComponent>(Component);
                if (ProceduralMesh)
                {
                    // Si un ProceduralMeshComponent est trouvé, on le détruit
                    UE_LOG(LogTemp, Warning, TEXT("Destroying attached ProceduralMesh"));

                    ProceduralMesh->DestroyComponent();
                    break; // On arrête dès qu'on a trouvé et détruit le maillage
                }
            }
        }

        // Assigner le nouveau maillage
        NewMesh = NewMeshToSet;

        // Attacher le nouveau maillage au MeshComponent (ou au RootComponent si nécessaire)
        if (MeshComponent)
        {
            NewMesh->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
        }
        else
        {
            NewMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        }

        // Réinitialiser la transformation relative
        NewMesh->SetRelativeTransform(FTransform::Identity);

        // Enregistrer le maillage et le rendre visible dans la scène
        NewMesh->RegisterComponent();
        NewMesh->MarkRenderStateDirty();

        // Assurer que le maillage est visible et que la collision est activée
        NewMesh->SetVisibility(true);
        NewMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
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
    if (MeshComponent)
    {
        // Recherche du premier ProceduralMeshComponent attaché à MeshComponent
        for (UActorComponent* Component : MeshComponent->GetAttachChildren())
        {
            UProceduralMeshComponent* ProceduralMesh = Cast<UProceduralMeshComponent>(Component);
            if (ProceduralMesh)
            {
                // Si un ProceduralMeshComponent est trouvé, on le détruit
                UE_LOG(LogTemp, Warning, TEXT("Destroying attached ProceduralMesh"));

                ProceduralMesh->DestroyComponent();
                break; // On arrête dès qu'on a trouvé et détruit le maillage
            }
        }
    }
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

FMeshData AHighlightableObject::GetVerticesAndTriangles()
{
    FMeshData MeshData;

    UStaticMeshComponent* TargetMeshComponent;
    if (TargetActor)
    {
        TargetMeshComponent = Cast<UStaticMeshComponent>(TargetActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    else
    {
        TargetMeshComponent = MeshComponent;
    }

    TMap<FVector, int32> VertexMap;
    if (TargetMeshComponent && TargetMeshComponent->GetStaticMesh())
    {
        UStaticMesh* StaticMesh = TargetMeshComponent->GetStaticMesh();
        FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
        if (RenderData)
        {
            for (int32 LODIndex = 0; LODIndex < RenderData->LODResources.Num(); ++LODIndex)
            {
                const FStaticMeshLODResources& LODResources = RenderData->LODResources[LODIndex];
                const FPositionVertexBuffer& VertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
                const FIndexArrayView& IndexBuffer = LODResources.IndexBuffer.GetArrayView();

                for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); ++i)
                {
                    FVector VertexPos = FVector(VertexBuffer.VertexPosition(i).X, VertexBuffer.VertexPosition(i).Y, VertexBuffer.VertexPosition(i).Z);
                    int32 NewIndex = MeshData.Vertices.Add(VertexPos);
                    VertexMap.Add(VertexPos, NewIndex);
                }

                const int32 NumIndices = IndexBuffer.Num();

                for (int32 i = 0; i < NumIndices; i += 3)
                {
                    if (i + 2 >= NumIndices)
                    {
                        break;
                    }

                    int32 Index0 = IndexBuffer[i];
                    int32 Index1 = IndexBuffer[i + 1];
                    int32 Index2 = IndexBuffer[i + 2];

                    if (Index0 < MeshData.Vertices.Num() && Index1 < MeshData.Vertices.Num() && Index2 < MeshData.Vertices.Num())
                    {
                        // Vérifier que les indices existent dans VertexMap
                        if (VertexMap.Contains(MeshData.Vertices[Index0]) &&
                            VertexMap.Contains(MeshData.Vertices[Index1]) &&
                            VertexMap.Contains(MeshData.Vertices[Index2]))
                        {
                            MeshData.Triangles.Add(VertexMap[MeshData.Vertices[Index0]]);
                            MeshData.Triangles.Add(VertexMap[MeshData.Vertices[Index1]]);
                            MeshData.Triangles.Add(VertexMap[MeshData.Vertices[Index2]]);
                        }
                    }
                }

            }
        }
    }

    return MeshData;
}


TArray<FVector> AHighlightableObject::GetVertices()
{

    UStaticMeshComponent* TargetMeshComponent;

    if (TargetActor)
    {
        TargetMeshComponent = Cast<UStaticMeshComponent>(TargetActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    }
    else
    {
        TargetMeshComponent = MeshComponent;
    }
    TArray<FVector> Vertices;
    if (TargetMeshComponent && TargetMeshComponent->GetStaticMesh())
    {
        UStaticMesh* StaticMesh = TargetMeshComponent->GetStaticMesh();
        FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
        if (RenderData)
        {
            for (int32 LODIndex = 0; LODIndex < RenderData->LODResources.Num(); ++LODIndex)
            {
                const FStaticMeshLODResources& LODResources = RenderData->LODResources[LODIndex];
                const FPositionVertexBuffer& VertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
                for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); ++i)
                {
                    FVector VertexPos = FVector(VertexBuffer.VertexPosition(i).X, VertexBuffer.VertexPosition(i).Y, VertexBuffer.VertexPosition(i).Z);
                    Vertices.Add(VertexPos);
                }
            }
        }
    }
    return Vertices;
}
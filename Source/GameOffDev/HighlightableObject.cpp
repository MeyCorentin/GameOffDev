#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "LampeTorche.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProceduralMeshComponent.h" 
#include "Kismet/KismetMathLibrary.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/SpotLight.h" 
#include "Engine/PointLight.h"
#include "LampeTorche.h"

// Sets default values
AHighlightableObject::AHighlightableObject()
{
    PrimaryActorTick.bCanEverTick = true;

    if (!TargetActor)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
        RootComponent = MeshComponent;
    }
    isDisplay = false;
}

bool AHighlightableObject::isIlluminatedBySpotLight()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    bool isIlluminated = false;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor)
        {
            continue;
        }

        USpotLightComponent* SpotLightComponent = Actor->FindComponentByClass<USpotLightComponent>();
        if (!SpotLightComponent)
        {
            continue;
        }

        FVector LightDirection = SpotLightComponent->GetForwardVector();
        FVector LightLocation = SpotLightComponent->GetComponentLocation();
        float Length = SpotLightComponent->AttenuationRadius;
        float ConeAngle = SpotLightComponent->InnerConeAngle;

        if (RequiredColor.R != SpotLightComponent->LightColor.R ||
            RequiredColor.G != SpotLightComponent->LightColor.G ||
            RequiredColor.B != SpotLightComponent->LightColor.B)
        {
            continue;
        }

        AActor* _actor = (TargetActor) ? TargetActor : this;
        isIlluminated = IsMeshInCone(_actor, Length, ConeAngle, LightDirection, LightLocation);

        if (isIlluminated)
        {
            break;  
        }
    }

    return isIlluminated;
}

bool AHighlightableObject::isIlluminatedByPointLight()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    bool isIlluminated = false;


    for (AActor* Actor : FoundActors)
    {
        UPointLightComponent* PointLightComponent = Actor->FindComponentByClass<UPointLightComponent>();
        if (!PointLightComponent ||
            RequiredColor.R != PointLightComponent->LightColor.R ||
            RequiredColor.G != PointLightComponent->LightColor.G ||
            RequiredColor.B != PointLightComponent->LightColor.B)
        {
            continue;
        }
        
        if (Actor->IsA(ASpotLight::StaticClass()) || Actor->IsA(ALampeTorche::StaticClass()))
        {
            continue;
        }
        FVector LightLocation = PointLightComponent->GetComponentLocation();
        float Radius = PointLightComponent->AttenuationRadius;
        TArray<FVector> Vertices = GetVertices();

        for (const FVector& Vertex : Vertices)
        {

            FVector WorldVertex0 = this->GetTransform().TransformPosition(Vertex);
            if (TargetActor != nullptr)
                WorldVertex0 = TargetActor->GetTransform().TransformPosition(Vertex);
            float distance = FVector::Dist(LightLocation, WorldVertex0);
            if (distance <= Radius)
            {
                return true;
            }
        }

        if (isIlluminated)
        {
            break;
        }
    }

    return isIlluminated;
}



bool AHighlightableObject::isIlluminated()
{
    bool bIlluminatedBySpotLight = isIlluminatedBySpotLight();
    bool bIlluminatedByPointLight = isIlluminatedByPointLight();
    if (bIlluminatedBySpotLight == true || bIlluminatedByPointLight == true)
        return true;

    return false;
}

    
bool AHighlightableObject::getDisplayStatus()
{
    return isDisplay;
}


void AHighlightableObject::HandleObjectStatus()
{
    if (isIlluminated())
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
    else {
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



bool AHighlightableObject::IsMeshInCone(AActor* actor, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation)
{

    bool bFoundTriangleInCone = false;
    TArray<FVector>  Vertices = GetVertices();
    for (int32 i = 0; i < Vertices.Num(); i += 1)
    {
        FVector Vertex0 = Vertices[i];
        FVector WorldVertex0 = actor->GetTransform().TransformPosition(Vertex0);
        bool bVertex0InCone = IsPointInCone(WorldVertex0, Length, ConeAngle, LightDirection, LightLocation);
        if (bVertex0InCone )
        {
            return true;
        }
    }
    return false;
}

bool AHighlightableObject::IsPointInCone(const FVector& Point,  float Length, float ConeAngle, FVector LightDirection, FVector LightLocation)
{
    FVector DirectionToPoint = (Point - LightLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(LightDirection, DirectionToPoint);
    float Angle = FMath::Acos(DotProduct) * 180.f / PI;  // Angle en degrés

    if (Angle <= ConeAngle)
    {
        float DistanceToPoint = FVector::Dist(LightLocation, Point);
        if (DistanceToPoint <= Length)
        {
            return true;
        }
    }

    return false;
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
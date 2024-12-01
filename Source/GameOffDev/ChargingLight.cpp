// Fill out your copyright notice in the Description page of Project Settings.


#include "ChargingLight.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AChargingLight::AChargingLight()
{
    PrimaryActorTick.bCanEverTick = true;
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    LightBulbe = CreateDefaultSubobject<UPointLightComponent>(TEXT("LightBulbe"));
    LightBulbe->SetIntensity(1000.0f);
    LightBulbe->AttenuationRadius = 0;
    LightBulbe->SetupAttachment(RootComponent);
}

void AChargingLight::UpdateIntensity()
{
    LightBulbe->SetIntensity(LightBulbe->Intensity - LightIntensityInit * DecreaseRate);
    LightBulbe->AttenuationRadius = LightBulbe->AttenuationRadius  - AttenuationRadiusInit * DecreaseRate;
}


// Called when the game starts or when spawned
void AChargingLight::BeginPlay()
{
	Super::BeginPlay();
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("UpdateIntensity"));


    if (GEngine)
    {
        UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
        if (World)
        {
            World->GetTimerManager().SetTimer(
                BatteryTimerHandle,
                TimerDelegate,
                1.0f,
                true
            );
        }
    }
	
}
// Called every frame
void AChargingLight::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (isIlluminatedBySpotLight())
    {
        UE_LOG(LogTemp, Warning, TEXT("LIGHT! IntensityInit: %f, AttenuationRadiusInit: %f"), LightIntensityInit, AttenuationRadiusInit);

        LightBulbe->SetIntensity(LightIntensityInit);
        LightBulbe->AttenuationRadius = AttenuationRadiusInit;
        LightBulbe->SetMobility(EComponentMobility::Movable);
        LightBulbe->MarkRenderStateDirty();

    }
}


bool AChargingLight::isIlluminatedBySpotLight()
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
        float ConeAngle = SpotLightComponent->InnerConeAngle;isIlluminated = IsMeshInCone(this, Length, ConeAngle, LightDirection, LightLocation);

        if (isIlluminated)
        {
            this->LightBulbe->LightColor = SpotLightComponent->LightColor;
            break;
        }
    }

    return isIlluminated;
}



bool AChargingLight::IsMeshInCone(AActor* actor, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation)
{

    bool bFoundTriangleInCone = false;
    TArray<FVector>  Vertices = GetVertices();
    for (int32 i = 0; i < Vertices.Num(); i += 1)
    {
        FVector Vertex0 = Vertices[i];
        FVector WorldVertex0 = actor->GetTransform().TransformPosition(Vertex0);
        bool bVertex0InCone = IsPointInCone(WorldVertex0, Length, ConeAngle, LightDirection, LightLocation);
        if (bVertex0InCone)
        {
            return true;
        }
    }
    return false;
}

bool AChargingLight::IsPointInCone(const FVector& Point, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation)
{
    FVector DirectionToPoint = (Point - LightLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(LightDirection, DirectionToPoint);
    float Angle = FMath::Acos(DotProduct) * 180.f / PI;

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


TArray<FVector> AChargingLight::GetVertices()
{
    UStaticMeshComponent* TargetMeshComponent = MeshComponent;
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



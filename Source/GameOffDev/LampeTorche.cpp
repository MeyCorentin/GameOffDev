#include "LampeTorche.h"
#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

// Sets default values
ALampeTorche::ALampeTorche()
{
    PrimaryActorTick.bCanEverTick = true;

    LampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMesh"));
    RootComponent = LampMesh;

    LampSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LampSpotLight"));
    LampSpotLight->SetupAttachment(LampMesh);

    LampSpotLight->SetIntensity(100000.f);
    LampSpotLight->SetLightColor(FLinearColor::White);
    LampSpotLight->SetAttenuationRadius(5000.f);
}

void ALampeTorche::BeginPlay()
{
    Super::BeginPlay();

    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("UpdateBattery"));


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
            UE_LOG(LogTemp, Warning, TEXT("Timer !"));
        }
    }
    InitBatteryLevel = BatteryLevel;
    InitialIntensity = LampSpotLight->Intensity;
    InitialAttenuationRadius = LampSpotLight->AttenuationRadius;
}

void ALampeTorche::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(0, 0.1f, FColor::White, FString::Printf(TEXT("%.0f%%"), BatteryLevel));
    }

}


void ALampeTorche::Charge(float energy)
{
    BatteryLevel += energy;
}

void ALampeTorche::ChangeColor(int32 ColorCode)
{
    switch (ColorCode)
    {
    case 1:
        LampSpotLight->SetLightColor(FLinearColor::Red);
        break;
    case 2:
        LampSpotLight->SetLightColor(FLinearColor::Green);
        break;
    case 3:
        LampSpotLight->SetLightColor(FLinearColor::Blue);
        break;
    default:
        //UE_LOG(LogTemp, Warning, TEXT("Invalid color code: %d. Use 1 (Red), 2 (Green), or 3 (Blue)."), ColorCode);
        break;
    }
}

void ALampeTorche::AttachToPlayer(USkeletalMeshComponent* PlayerMesh)
{
    if (PlayerMesh)
    {
        AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, "HandSocket");
    }
}

void ALampeTorche::UpdateBattery()
{
    if (BatteryLevel > 0.f && InitBatteryLevel > 0.f)
    {
        float BatteryDrainAmount = (BatteryDrainRate / 100.f) * InitBatteryLevel;
        BatteryLevel = FMath::Max(BatteryLevel - BatteryDrainAmount, 0.f);
        float BatteryProportion = BatteryLevel / InitBatteryLevel;
        LampSpotLight->SetAttenuationRadius(InitialAttenuationRadius * BatteryProportion);
        LampSpotLight->SetIntensity(InitialIntensity * BatteryProportion);

        if (BatteryLevel <= 0.f)
        {
            LampSpotLight->SetIntensity(0.f);
        }
    }
}

bool ALampeTorche::IsActorInDetectionCone(AActor* TargetActor, FColor RequiredColor)
{
    if (!TargetActor || !LampSpotLight)
    {
        return false;
    }

    // Récupérer la portée (longueur) et l'angle du cône de lumière
    float Length = LampSpotLight->AttenuationRadius;
    float ConeAngle = LampSpotLight->OuterConeAngle;

    // Vérifier que l'objet ciblé est de type AHighlightableObject et possède un StaticMeshComponent
    AHighlightableObject* HighlightableObject = Cast<AHighlightableObject>(TargetActor);
    UStaticMeshComponent* StaticMeshComponent = TargetActor->FindComponentByClass<UStaticMeshComponent>();
    if (!StaticMeshComponent)
    {
        return false;
    }

    UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
    if (!StaticMesh || RequiredColor != LampSpotLight->LightColor)
    {
        return false;
    }

    FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
    if (!RenderData)
    {
        return false;
    }

    FVector LampPosition = LampSpotLight->GetComponentLocation();
    FVector LampDirection = LampSpotLight->GetForwardVector();

    FTransform ComponentTransform = StaticMeshComponent->GetComponentTransform();

    for (int32 LODIndex = 0; LODIndex < RenderData->LODResources.Num(); ++LODIndex)
    {
        const FStaticMeshLODResources& LODResources = RenderData->LODResources[LODIndex];
        const FPositionVertexBuffer& VertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;

        for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); ++i)
        {
            FVector VertexPos = FVector(VertexBuffer.VertexPosition(i).X, VertexBuffer.VertexPosition(i).Y, VertexBuffer.VertexPosition(i).Z);
            FVector WorldVertexPos = ComponentTransform.TransformPosition(VertexPos);

            FVector DirectionToVertex = (WorldVertexPos - LampPosition).GetSafeNormal();

            float DotProduct = FVector::DotProduct(LampDirection, DirectionToVertex);
            float Angle = FMath::Acos(DotProduct) * 180.f / PI;

            if (Angle <= ConeAngle)
            {
                float DistanceToVertex = FVector::Dist(LampPosition, WorldVertexPos);
                if (DistanceToVertex <= Length)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

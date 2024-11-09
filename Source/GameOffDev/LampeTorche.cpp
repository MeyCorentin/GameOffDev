#include "LampeTorche.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h" // Inclure pour USkeletalMeshComponent
#include "GameFramework/Actor.h" // Pour AttachToComponent
#include "Components/ActorComponent.h"

// Sets default values
ALampeTorche::ALampeTorche()
{
    PrimaryActorTick.bCanEverTick = true;

    // Créer un mesh pour la lampe torche
    LampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMesh"));
    RootComponent = LampMesh;

    // Créer un spot light pour la lampe torche
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
        UE_LOG(LogTemp, Warning, TEXT("Invalid color code: %d. Use 1 (Red), 2 (Green), or 3 (Blue)."), ColorCode);
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

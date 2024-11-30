#include "LampeTorche.h"
#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"


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

    _ColorFilter[DefaultColor] = true;

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
    InitBatteryLevel = BatteryLevel;
    InitialIntensity = LampSpotLight->Intensity;
    InitialAttenuationRadius = LampSpotLight->AttenuationRadius;

    FLinearColor LampColor;

    switch (DefaultColor)
    {
        case 0:
            LampColor = FLinearColor(0.0f, 0.0f, 1.0f); // Bleu
            break;
        case 1:
            LampColor = FLinearColor(1.0f, 1.0f, 0.0f);
            break;
        case 2:
            LampColor = FLinearColor(1.0f, 0.0f, 0.0f);
            break;
        case 3:
            LampColor = FLinearColor(0.0f, 1.0f, 0.0f);
            break;
        case 4:
            LampColor = FLinearColor(1.0f, 1.0f, 1.0f);
            break;
        default:
            LampColor = FLinearColor(1.0f, 1.0f, 1.0f);
            break;
    }

    if (LampSpotLight)
    {
        LampSpotLight->SetLightColor(LampColor);
    }
}

void ALampeTorche::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void ALampeTorche::Charge(float energy)
{
    BatteryLevel += energy;
    if (BatteryLevel > 100)
        BatteryLevel = 100;
}

void ALampeTorche::ChangeColor(int32 ColorCode)
{
    if (ColorCode < 0 || ColorCode > _ColorFilter.Num())
    {
        return;
    }

    switch (ColorCode)
    {
        case 0:
            if (_ColorFilter[0])
            {
                LampSpotLight->SetLightColor(FLinearColor::Blue);
                DefaultColor = 0;
            }
            break;

        case 1:
            if (_ColorFilter[1])
            {
                LampSpotLight->SetLightColor(FLinearColor::Yellow);
                DefaultColor = 1;
            }
            break;

        case 2:
            if (_ColorFilter[2])
            {
                LampSpotLight->SetLightColor(FLinearColor::Red);
                DefaultColor = 2;
            }
            break;

        case 3:
            if (_ColorFilter[3])
            {
                LampSpotLight->SetLightColor(FLinearColor::Green);
                DefaultColor = 3;
            }
            break;

        default:
            break;
    }

}

void ALampeTorche::PicktupColor(int32 FilterValue)
{
    if (FilterValue >= 0 && FilterValue < _ColorFilter.Num())
    {
        _ColorFilter[FilterValue] = true;
        if (DefaultColor == 4)
        {
            ChangeColor(_ColorFilter[FilterValue]);
        }
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

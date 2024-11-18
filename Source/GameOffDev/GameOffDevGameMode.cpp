// GameOffDevGameMode.cpp
#include "GameOffDevGameMode.h"
#include "GameOffDevCharacter.h"
#include "GameOffDevPlayerController.h"
#include "EngineUtils.h"

#include "Engine/World.h"
#include "Engine/SpotLight.h"
#include "Engine/PointLight.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"

AGameOffDevGameMode::AGameOffDevGameMode()
{
    // Définir la classe de Pawn par défaut à notre personnage Blueprinté
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // Définir le PlayerController personnalisé
    PlayerControllerClass = AGameOffDevPlayerController::StaticClass();
}

void AGameOffDevGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Vérification toutes les 5 secondes
    GetWorldTimerManager().SetTimer(CheckLightsTimerHandle, this, &AGameOffDevGameMode::CheckLightsAndReturnToTitle, 1.0f, true);
}

void AGameOffDevGameMode::CheckLightsAndReturnToTitle()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    bool bLightFound = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        TArray<UActorComponent*> Components;
        Actor->GetComponents(Components);

        for (UActorComponent* Component : Components)
        {
            if (USpotLightComponent* SpotLightComponent = Cast<USpotLightComponent>(Component))
            {

                if (SpotLightComponent->AttenuationRadius > 5.f ||
                    SpotLightComponent->Intensity > 5.f)
                {
                    bLightFound = true;
                    break;
                }
            }
            if (UPointLightComponent* PointLightComponent = Cast<UPointLightComponent>(Component))
            {
                if (PointLightComponent->AttenuationRadius > 5.f ||
                    PointLightComponent->Intensity > 5.f)
                {
                    bLightFound = true;
                    break;
                }
            }
        }

        if (bLightFound)
        {
            break;
        }
    }
    if (!bLightFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid lights found! Returning to title screen..."));
        UGameplayStatics::OpenLevel(World, FName("MainMenuMap"));
    }
}

// GameOffDevGameMode.cpp
#include "GameOffDevGameMode.h"
#include "GameOffDevCharacter.h"
#include "GameOffDevPlayerController.h" // Ajoutez cette ligne pour inclure votre PlayerController

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

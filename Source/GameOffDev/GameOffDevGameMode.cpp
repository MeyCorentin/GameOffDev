// GameOffDevGameMode.cpp
#include "GameOffDevGameMode.h"
#include "GameOffDevCharacter.h"
#include "GameOffDevPlayerController.h" // Ajoutez cette ligne pour inclure votre PlayerController

AGameOffDevGameMode::AGameOffDevGameMode()
{
    // D�finir la classe de Pawn par d�faut � notre personnage Blueprint�
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // D�finir le PlayerController personnalis�
    PlayerControllerClass = AGameOffDevPlayerController::StaticClass();
}

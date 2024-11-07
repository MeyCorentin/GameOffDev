#include "GameOffDevPlayerController.h"

void AGameOffDevPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Active le curseur dans le PlayerController
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    SetInputMode(FInputModeGameAndUI()); // Permet le contrôle du jeu et de l'UI
}
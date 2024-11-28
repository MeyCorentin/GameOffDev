// MyGameInstance.cpp
#include "GameOffGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UGameOffGameInstance::SaveGameData()
{
    if (!CurrentSave)
    {
        CurrentSave = Cast<UGameOffSaveGame>(UGameplayStatics::CreateSaveGameObject(UGameOffSaveGame::StaticClass()));
    }

    // Sauvegarder la sauvegarde actuelle dans un slot
    UGameplayStatics::SaveGameToSlot(CurrentSave, TEXT("SaveSlot"), 0);
}

void UGameOffGameInstance::LoadGameData()
{
    if (UGameplayStatics::DoesSaveGameExist(TEXT("SaveSlot"), 0))
    {
        CurrentSave = Cast<UGameOffSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SaveSlot"), 0));
    }
    else
    {
        // Créer une nouvelle sauvegarde si aucune n'existe
        CurrentSave = Cast<UGameOffSaveGame>(UGameplayStatics::CreateSaveGameObject(UGameOffSaveGame::StaticClass()));
    }
}

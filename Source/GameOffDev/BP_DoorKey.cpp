#include "BP_DoorKey.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Constructeur
ABP_DoorKey::ABP_DoorKey()
{
    // Initialisation par défaut
    bKeyCollected = false;
    DoorInstance = nullptr;
    KeyInstance = nullptr;
}

// Fonction pour initialiser la porte et la clé dans le monde
void ABP_DoorKey::InitializeDoorAndKey()
{
    // Vérifier si DoorClass et KeyClass ont été définies dans l'éditeur
    if (DoorClass && KeyClass)
    {
        // Spawner la porte et la clé dans le monde à la position de l'acteur BP_DoorKey
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        // Spawner la porte
        DoorInstance = GetWorld()->SpawnActor<AActor>(DoorClass, GetActorLocation(), GetActorRotation(), SpawnParams);

        // Spawner la clé
        KeyInstance = GetWorld()->SpawnActor<AActor>(KeyClass, GetActorLocation() + FVector(100.f, 0.f, 0.f), GetActorRotation(), SpawnParams);  // Ajuster la position si nécessaire
    }
}

// Fonction pour ramasser la clé
void ABP_DoorKey::CollectKey()
{
    if (KeyInstance)
    {
        // Détruire la clé ou la cacher
        KeyInstance->Destroy();
        bKeyCollected = true;
    }
}

// Fonction pour ouvrir la porte si la clé a été ramassée
void ABP_DoorKey::InteractWithDoor()
{
    if (bKeyCollected && DoorInstance)
    {
        // Appeler la fonction OpenDoor sur la porte
        UFunction* OpenDoorFunction = DoorInstance->FindFunction(TEXT("OpenDoor"));
        if (OpenDoorFunction)
        {
            DoorInstance->ProcessEvent(OpenDoorFunction, nullptr);
        }
    }
    else
    {
        // Si la clé n'a pas été ramassée, afficher un message
        UE_LOG(LogTemp, Warning, TEXT("La clé doit être ramassée avant d'ouvrir la porte !"));
    }
}

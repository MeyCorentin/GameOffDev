#include "BP_DoorKey.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Constructeur
ABP_DoorKey::ABP_DoorKey()
{
    // Initialisation par d�faut
    bKeyCollected = false;
    DoorInstance = nullptr;
    KeyInstance = nullptr;
}

// Fonction pour initialiser la porte et la cl� dans le monde
void ABP_DoorKey::InitializeDoorAndKey()
{
    // V�rifier si DoorClass et KeyClass ont �t� d�finies dans l'�diteur
    if (DoorClass && KeyClass)
    {
        // Spawner la porte et la cl� dans le monde � la position de l'acteur BP_DoorKey
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        // Spawner la porte
        DoorInstance = GetWorld()->SpawnActor<AActor>(DoorClass, GetActorLocation(), GetActorRotation(), SpawnParams);

        // Spawner la cl�
        KeyInstance = GetWorld()->SpawnActor<AActor>(KeyClass, GetActorLocation() + FVector(100.f, 0.f, 0.f), GetActorRotation(), SpawnParams);  // Ajuster la position si n�cessaire
    }
}

// Fonction pour ramasser la cl�
void ABP_DoorKey::CollectKey()
{
    if (KeyInstance)
    {
        // D�truire la cl� ou la cacher
        KeyInstance->Destroy();
        bKeyCollected = true;
    }
}

// Fonction pour ouvrir la porte si la cl� a �t� ramass�e
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
        // Si la cl� n'a pas �t� ramass�e, afficher un message
        UE_LOG(LogTemp, Warning, TEXT("La cl� doit �tre ramass�e avant d'ouvrir la porte !"));
    }
}

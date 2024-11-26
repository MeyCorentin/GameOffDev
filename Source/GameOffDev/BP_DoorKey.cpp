#include "BP_DoorKey.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ABP_DoorKey::ABP_DoorKey()
{
    bKeyCollected = false;
    DoorInstance = nullptr;
    KeyInstance = nullptr;
}

void ABP_DoorKey::InitializeDoorAndKey()
{
    if (DoorClass && KeyClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        DoorInstance = GetWorld()->SpawnActor<AActor>(DoorClass, GetActorLocation(), GetActorRotation(), SpawnParams);

        KeyInstance = GetWorld()->SpawnActor<AActor>(KeyClass, GetActorLocation() + FVector(100.f, 0.f, 0.f), GetActorRotation(), SpawnParams);
    }
}

void ABP_DoorKey::CollectKey()
{
    if (KeyInstance)
    {
        KeyInstance->Destroy();
        bKeyCollected = true;
    }
}

void ABP_DoorKey::InteractWithDoor()
{
    if (bKeyCollected && DoorInstance)
    {
        UFunction* OpenDoorFunction = DoorInstance->FindFunction(TEXT("OpenDoor"));
        if (OpenDoorFunction)
        {
            DoorInstance->ProcessEvent(OpenDoorFunction, nullptr);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("La clé doit être ramassée avant d'ouvrir la porte !"));
    }
}

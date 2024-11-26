#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BP_DoorKey.generated.h"

UCLASS()
class GAMEOFFDEV_API ABP_DoorKey : public AActor
{
    GENERATED_BODY()

public:
    ABP_DoorKey();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TSubclassOf<AActor> DoorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TSubclassOf<AActor> KeyClass;

    AActor* DoorInstance;
    AActor* KeyInstance;

    bool bKeyCollected;

public:
    void InitializeDoorAndKey();

    void CollectKey();

    void InteractWithDoor();
};

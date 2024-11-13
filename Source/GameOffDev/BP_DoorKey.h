#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BP_DoorKey.generated.h"

UCLASS()
class GAMEOFFDEV_API ABP_DoorKey : public AActor
{
    GENERATED_BODY()

public:
    // Constructeur
    ABP_DoorKey();

protected:
    // Référence à la classe de la porte
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TSubclassOf<AActor> DoorClass;

    // Référence à la classe de la clé
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TSubclassOf<AActor> KeyClass;

    // Références aux instances de la porte et de la clé
    AActor* DoorInstance;
    AActor* KeyInstance;

    // Si la clé a été ramassée
    bool bKeyCollected;

public:
    // Fonction pour initialiser la porte et la clé dans le monde
    void InitializeDoorAndKey();

    // Fonction pour ramasser la clé
    void CollectKey();

    // Fonction pour ouvrir la porte si la clé a été ramassée
    void InteractWithDoor();
};

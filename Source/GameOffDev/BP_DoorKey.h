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
    // R�f�rence � la classe de la porte
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TSubclassOf<AActor> DoorClass;

    // R�f�rence � la classe de la cl�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TSubclassOf<AActor> KeyClass;

    // R�f�rences aux instances de la porte et de la cl�
    AActor* DoorInstance;
    AActor* KeyInstance;

    // Si la cl� a �t� ramass�e
    bool bKeyCollected;

public:
    // Fonction pour initialiser la porte et la cl� dans le monde
    void InitializeDoorAndKey();

    // Fonction pour ramasser la cl�
    void CollectKey();

    // Fonction pour ouvrir la porte si la cl� a �t� ramass�e
    void InteractWithDoor();
};

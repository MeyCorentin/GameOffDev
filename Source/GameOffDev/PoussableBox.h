// PoussableBox.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoussableBox.generated.h"

UCLASS()
class GAMEOFFDEV_API APoussableBox : public AActor
{
    GENERATED_BODY()

public:
    APoussableBox();

protected:
    virtual void BeginPlay() override;

public:
    // Fonction pour pousser ou tirer la boîte dans une direction donnée
    void MoveInDirection(const FVector& Direction, float Force);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* BoxMesh;
};

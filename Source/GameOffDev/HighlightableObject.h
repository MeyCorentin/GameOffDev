#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LampeTorche.h"
#include "HighlightableObject.generated.h"

// Forward declaration of AFlashlight class
class AFlashlight;
class UStaticMeshComponent;

UCLASS()
class GAMEOFFDEV_API AHighlightableObject : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AHighlightableObject();

    bool getDisplayStatus();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FColor RequiredColor = FColor(255, 255, 255);

protected:
    // Mesh component representing the visible part of the object
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    AActor* TargetActor;

    // Called every frame to check visibility based on flashlight illumination
    virtual void Tick(float DeltaTime) override;

    // Checks if the object is within the flashlight's cone of illumination
    bool IsIlluminatedByFlashlight(ALampeTorche* Flashlight);

    void HandleObjectStatus();

    void DisplayObject();


    bool CheckCollisionWithPlayer(UStaticMeshComponent* MeshComponent);

    void HideObject();

    FVector GetWorldVertexPosition(int32 LODIndex, int32 VertexIndex) const;

    bool isDisplay;
};

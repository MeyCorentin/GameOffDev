#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FColor RequiredColor = FColor(255, 255, 255);


    bool getDisplayStatus();

protected:

    bool isDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    bool Fusion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    int _rf;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    int _gf;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    int _bf;

    virtual void Tick(float DeltaTime) override;

    TArray<FVector> GetVertices();

    bool isIlluminated();
    bool isIlluminatedBySpotLight();
    bool isIlluminatedByPointLight();
    bool isIlluminatedByPointLightFusion();

    void HandleObjectStatus();

    bool CheckCollisionWithPlayer(UStaticMeshComponent* MeshComponent);

    void HideObject();

    void DisplayObject();

    bool IsMeshInCone(AActor* actor, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation);

    bool IsPointInCone(const FVector& Point, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation);

};

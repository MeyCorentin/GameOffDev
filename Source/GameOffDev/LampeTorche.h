#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/SpotLightComponent.h>
#include "HighlightableObject.h"
#include "LampeTorche.generated.h"

// Déclaration de la classe ALampeTorche
UCLASS()
class GAMEOFFDEV_API ALampeTorche : public AActor
{
    GENERATED_BODY()

public:
    ALampeTorche();

protected:
    virtual void BeginPlay() override;

    float InitBatteryLevel;
    float InitialIntensity;
    float InitialAttenuationRadius;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* LampMesh;

    UPROPERTY(VisibleAnywhere)
    USpotLightComponent* LampSpotLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float BatteryLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float BatteryDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float LightRadiusFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float LightIntensityFactor;

    void AttachToPlayer(class USkeletalMeshComponent* PlayerMesh);

    void ChangeColor(int32 ColorCode);

    void Charge(float energy);

    FTimerHandle BatteryTimerHandle;

    UFUNCTION()
    void UpdateBattery();
    TArray<FVector> MoveTrianglePointsToConeLimit(const FVector& WorldVertex0, const FVector& WorldVertex1, const FVector& WorldVertex2, const FVector& ConeDirection, float ConeAngle);
    void ProcessTrianglesInCone(const FVector& WorldVertex0, const FVector& WorldVertex1, const FVector& WorldVertex2, const FVector& LightDirection, float ConeAngle, TArray<FVector>& VerticesInCone, TArray<int32>& TrianglesInCone, const FTransform& ChildTransform);
    bool IsActorInDetectionCone(AActor* Parent, AActor* Child, FColor RequiredColor);
    FVector GetPointOnConeLimit(const FVector& Point, const FVector& InsidePoint);
    TArray<bool> CheckPointsInCone(const TArray<FVector>& Points, const FVector& ConeDirection, float ConeAngle);
    bool IsAllPointsInCone(const TArray<bool>& PointsInCone);
    int32 FindInsidePointIndex(const TArray<bool>& PointsInCone);
    FVector MovePointToConeLimit(const FVector& OutsidePoint, const FVector& InsidePoint, const FVector& ConeDirection, float ConeAngle);
    bool IsPointInCone(const FVector& Point);
};


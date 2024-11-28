#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "MovingActorOnSpline.generated.h"

#pragma once


UCLASS()
class AMovingActorOnSpline : public AActor
{
    GENERATED_BODY()

public:
    AMovingActorOnSpline();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    USplineComponent* PathSpline;

    // Permet de sp�cifier la classe d'acteur � attacher dans l'�diteur
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    USceneComponent* ComponentToMove;

private:
    AActor* SpawnedActor; // L'acteur spawn� qui sera d�plac� le long de la spline
    float DistanceAlongSpline = 0.0f;
    float Speed = 200.0f; // Vitesse de d�placement
};  
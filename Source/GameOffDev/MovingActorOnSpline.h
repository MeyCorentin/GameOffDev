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

    // Permet de spécifier la classe d'acteur à attacher dans l'éditeur
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    USceneComponent* ComponentToMove;

private:
    AActor* SpawnedActor; // L'acteur spawné qui sera déplacé le long de la spline
    float DistanceAlongSpline = 0.0f;
    float Speed = 200.0f; // Vitesse de déplacement
};  
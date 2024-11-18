#include "MovingActorOnSpline.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

AMovingActorOnSpline::AMovingActorOnSpline()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialisation du composant de spline
    PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
    RootComponent = PathSpline;
    ComponentToMove = CreateDefaultSubobject<USceneComponent>(TEXT("PointLight1"));
}

void AMovingActorOnSpline::BeginPlay()
{
    Super::BeginPlay();

    // V�rifie si un composant � d�placer est assign�
    if (ComponentToMove)
    {
        // Attache le composant � la spline � la position initiale
        ComponentToMove->AttachToComponent(PathSpline, FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void AMovingActorOnSpline::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // V�rifie si le composant � d�placer est bien attach�
    if (ComponentToMove && PathSpline)
    {
        // Incr�mente la distance le long de la spline
        DistanceAlongSpline += Speed * DeltaTime;

        // Boucle sur la longueur totale de la spline
        float SplineLength = PathSpline->GetSplineLength();
        if (DistanceAlongSpline > SplineLength)
        {
            DistanceAlongSpline = 0.0f;
        }

        // Obtient la nouvelle position et rotation le long de la spline
        FVector NewLocation = PathSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
        FRotator NewRotation = PathSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

        // Applique la position et la rotation au composant
        ComponentToMove->SetWorldLocation(NewLocation);
        ComponentToMove->SetWorldRotation(NewRotation);
    }
}

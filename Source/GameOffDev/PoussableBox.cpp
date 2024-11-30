// PoussableBox.cpp
#include "PoussableBox.h"

// Called when the game starts or when spawned
void APoussableBox::BeginPlay()
{
    Super::BeginPlay();
}

APoussableBox::APoussableBox()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialiser la boîte poussable avec un StaticMeshComponent
    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    RootComponent = BoxMesh;

    // Permettre la physique
    BoxMesh->SetSimulatePhysics(true);
}

void APoussableBox::MoveInDirection(const FVector& Direction, float Force)
{
    // Ajouter une force dans la direction donnée
    BoxMesh->AddForce(Direction * Force, NAME_None, true);
}

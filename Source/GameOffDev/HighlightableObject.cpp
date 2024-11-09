#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h" 
#include "GameOffDevCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AHighlightableObject::AHighlightableObject()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create the mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    isDisplay = false;
}

bool AHighlightableObject::IsIlluminatedByFlashlight(ALampeTorche* Flashlight)
{
    AGameOffDevCharacter* Character = Cast<AGameOffDevCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
    bool isActorDetected = Character->IsActorInDetectionCone(this);
    return isActorDetected;
}


bool AHighlightableObject::getDisplayStatus()
{
    return isDisplay;
}


void AHighlightableObject::HandleObjectStatus()
{
    ALampeTorche* Flashlight = nullptr;
    for (TActorIterator<ALampeTorche> It(GetWorld()); It; ++It)
    {
        Flashlight = *It;
        break;
    }

    if (Flashlight && IsIlluminatedByFlashlight(Flashlight))
    {
        this->DisplayObject();
    }
    else
    {
        this->HideObject();
    }

}


void AHighlightableObject::DisplayObject()
{
    MeshComponent->SetVisibility(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    isDisplay = true;

}

void AHighlightableObject::HideObject()
{
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    isDisplay = false;

}

void AHighlightableObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    HandleObjectStatus();
}

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
}

bool AHighlightableObject::IsIlluminatedByFlashlight(ALampeTorche* Flashlight)
{
    AGameOffDevCharacter* Character = Cast<AGameOffDevCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
    return Character->IsActorInDetectionCone(this);
}


void AHighlightableObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Trouver la première instance de lampe torche dans le monde
    ALampeTorche* Flashlight = nullptr;
    for (TActorIterator<ALampeTorche> It(GetWorld()); It; ++It)
    {
        Flashlight = *It;
        break; // Sortir après avoir trouvé la première lampe torche
    }

    if (Flashlight && IsIlluminatedByFlashlight(Flashlight))
    {
        MeshComponent->SetVisibility(true);
    }
    else
    {
        MeshComponent->SetVisibility(false);
    }
}

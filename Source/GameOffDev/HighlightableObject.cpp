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

    // Cr�e le composant de maillage uniquement si aucun TargetActor n'est assign�
    if (!TargetActor)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
        RootComponent = MeshComponent;
    }
    isDisplay = false;
}

bool AHighlightableObject::IsIlluminatedByFlashlight(ALampeTorche* Flashlight)
{
    // R�cup�rer le personnage du jeu
    AGameOffDevCharacter* Character = Cast<AGameOffDevCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

    // Si TargetActor est d�fini, v�rifier la d�tection sur lui, sinon v�rifier sur cet acteur
    if (Character)
    {
        if (TargetActor)
        {
            return Character->IsActorInDetectionCone(TargetActor, RequiredColor);
        }
        else
        {
            return Character->IsActorInDetectionCone(this, RequiredColor);
        }
    }

    return false;
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
        DisplayObject();
    }
    else
    {
        HideObject();
    }
}

void AHighlightableObject::DisplayObject()
{
    // Utiliser TargetActor si d�fini, sinon MeshComponent
    if (TargetActor)
    {
        TargetActor->SetActorHiddenInGame(false);
        TargetActor->SetActorEnableCollision(true);
    }
    else
    {
        MeshComponent->SetVisibility(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    isDisplay = true;
}

void AHighlightableObject::HideObject()
{
    // Utiliser TargetActor si d�fini, sinon MeshComponent
    if (TargetActor)
    {
        TargetActor->SetActorHiddenInGame(true);
        TargetActor->SetActorEnableCollision(false);
        TargetActor->SetHidden(true);
    }
    else
    {
        MeshComponent->SetVisibility(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    isDisplay = false;
}

void AHighlightableObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    HandleObjectStatus();
}

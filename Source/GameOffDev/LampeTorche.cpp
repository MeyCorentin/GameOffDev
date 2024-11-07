#include "LampeTorche.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h" // Inclure pour USkeletalMeshComponent
#include "GameFramework/Actor.h" // Pour AttachToComponent
#include "Components/ActorComponent.h"

// Sets default values
ALampeTorche::ALampeTorche()
{
    PrimaryActorTick.bCanEverTick = true;

    // Cr�er un mesh pour la lampe torche
    LampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMesh"));
    RootComponent = LampMesh;

    // Cr�er un spot light pour la lampe torche
    LampSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LampSpotLight"));
    LampSpotLight->SetupAttachment(LampMesh);  // Attache le spot light au mesh de la lampe

    // R�glages du spot light (par exemple, augmenter la port�e et l'intensit�)
    LampSpotLight->SetIntensity(100000.f);
    LampSpotLight->SetLightColor(FLinearColor::White);
    LampSpotLight->SetAttenuationRadius(5000.f);
}

// Called when the game starts or when spawned
void ALampeTorche::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ALampeTorche::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Attacher la lampe torche � la main du joueur
void ALampeTorche::AttachToPlayer(USkeletalMeshComponent* PlayerMesh)
{
    if (PlayerMesh)
    {
        // Attacher la lampe torche � la main du joueur (socket "HandSocket")
        AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, "HandSocket");
    }
}

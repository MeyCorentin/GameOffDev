#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/SpotLightComponent.h>
#include "LampeTorche.generated.h"

// Déclaration de la classe ALampeTorche
UCLASS()
class GAMEOFFDEV_API ALampeTorche : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ALampeTorche();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Composant de maillage pour la lampe torche
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* LampMesh;

    // Composant SpotLight pour la lampe torche
    UPROPERTY(VisibleAnywhere)
    USpotLightComponent* LampSpotLight;

    // Fonction pour attacher la lampe torche à la main du joueur
    void AttachToPlayer(class USkeletalMeshComponent* PlayerMesh);
};

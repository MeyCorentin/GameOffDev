#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/SpotLightComponent.h>
#include "LampeTorche.generated.h"

// D�claration de la classe ALampeTorche
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

    float InitBatteryLevel;
    float InitialIntensity;
    float InitialAttenuationRadius;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Composant de maillage pour la lampe torche
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* LampMesh;

    // Composant SpotLight pour la lampe torche
    UPROPERTY(VisibleAnywhere)
    USpotLightComponent* LampSpotLight;

    // Batterie de la lampe torche (de 0 � 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float BatteryLevel;

    // Consommation de la batterie par seconde
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float BatteryDrainRate;

    // Facteur de diminution de la port�e de la lampe en fonction de la batterie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float LightRadiusFactor;

    // Facteur de diminution de l'intensit� de la lumi�re en fonction de la batterie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp Battery")
    float LightIntensityFactor;

    // Fonction pour attacher la lampe torche � la main du joueur
    void AttachToPlayer(class USkeletalMeshComponent* PlayerMesh);

    void ChangeColor(int32 ColorCode);

    void Charge(float energy);

    FTimerHandle BatteryTimerHandle;

    UFUNCTION()
    void UpdateBattery();
};


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LampeTorche.h"
#include "Battery.generated.h"

UCLASS()
class GAMEOFFDEV_API ABattery : public AActor
{
	GENERATED_BODY()
	
public:	
	ABattery();

	float GetEnergyValue();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery")
	float energy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BatteryMesh;

	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;

};

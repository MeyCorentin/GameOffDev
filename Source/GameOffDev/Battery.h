// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	ABattery();

	float GetEnergyValue();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery")
	float energy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BatteryMesh;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

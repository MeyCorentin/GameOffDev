// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Engine/PointLight.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChargingLight.generated.h"

UCLASS()
class GAMEOFFDEV_API AChargingLight : public AActor
{
	GENERATED_BODY()
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	AChargingLight();
	bool isIlluminatedBySpotLight();
	bool IsMeshInCone(AActor* actor, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation);
	bool IsPointInCone(const FVector& Point, float Length, float ConeAngle, FVector LightDirection, FVector LightLocation);
	TArray<FVector> GetVertices();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* MeshComponent;


	FTimerHandle BatteryTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UPointLightComponent* LightBulbe;

	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float LightIntensityInit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float DecreaseRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float AttenuationRadiusInit;

	UFUNCTION()
	void UpdateIntensity();
};

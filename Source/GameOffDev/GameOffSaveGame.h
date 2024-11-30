// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameOffSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class GAMEOFFDEV_API UGameOffSaveGame : public USaveGame
{
	GENERATED_BODY()
    public:


        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        FVector PlayerPosition;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        TArray<bool> FilterInventory;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        FString PreviousLevelName;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        int DefaultColor;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        FLinearColor LightColor;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float BatteryLevel;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float BatteryDrainRate;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float LightRadiusFactor;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float LightIntensityFactor;
        
        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float InitBatteryLevel;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float InitialIntensity;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float InitialAttenuationRadius;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        float InnerConeAngle;

        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        bool CanEscape;


};

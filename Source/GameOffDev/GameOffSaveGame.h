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
};

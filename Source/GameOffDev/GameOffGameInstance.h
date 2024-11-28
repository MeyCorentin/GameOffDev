// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameOffSaveGame.h"
#include "GameOffGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMEOFFDEV_API UGameOffGameInstance : public UGameInstance
{
	GENERATED_BODY()


    public:
        UPROPERTY(BlueprintReadWrite, Category = "SaveData")
        UGameOffSaveGame* CurrentSave;

        // Sauvegarder les données
        UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        void SaveGameData();

        // Charger les données
        UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        void LoadGameData();
	
};

	// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameOffDevGameMode.generated.h"

UCLASS(minimalapi)
class AGameOffDevGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGameOffDevGameMode();
	void CheckLightsAndReturnToTitle();
	void BeginPlay();

	FTimerHandle CheckLightsTimerHandle;
};




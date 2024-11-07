#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameOffDevPlayerController.generated.h"

/**
 *
 */
UCLASS()
class GAMEOFFDEV_API AGameOffDevPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
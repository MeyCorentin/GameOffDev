// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LampeTorche.h"
#include "PoussableBox.h"
#include "GameOffDevCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AGameOffDevCharacter : public ACharacter
{
	GENERATED_BODY()
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PushOrPullAction;

public:
	AGameOffDevCharacter();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LampeTorche")
	ALampeTorche* CurrentLampeTorche;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PoussableBox")
	APoussableBox* TargetBox;


	bool IsActorInDetectionCone(AActor* TargetActor);

	bool bIsPushingOrPulling = false;
protected:


	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void DrawDetectionConeToMouse();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	FVector GetMouseWorldLocation() const;

	void FaceMouseCursor();

	void TraceToMouseCursor();

	void CheckForLampeTorche();

	void PickupLampeTorche(ALampeTorche* LampeTorche);
	
	void BeginPushOrPull();

	void EndPushOrPull();

	bool CheckForPushableBox();
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
};


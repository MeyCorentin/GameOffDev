// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LampeTorche.h"
#include "Battery.h"
#include "PoussableBox.h"
#include "Blueprint/UserWidget.h"
#include "HighlightableObject.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchColorAction1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchColorAction2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchColorAction3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DebugAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DropAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractionAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WheelAction;

public:

	int key_number = 0;

	AGameOffDevCharacter();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LampeTorche")
	ALampeTorche* CurrentLampeTorche;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PoussableBox")
	APoussableBox* TargetBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> BatteryWidgetClass;

	AActor* KeyRing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyRing")
	TSubclassOf<AActor> MyKeyRingBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebugModeEnabled = true;

	bool IsActorInDetectionCone(AActor* TargetActor, FColor RequiredColor);

	bool bIsPushingOrPulling = false;

	UUserWidget* BatteryWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> ColorWheelWidgetClass;
	UUserWidget* ColorWheelWidget;

	UFUNCTION(BlueprintCallable, Category = "Mouse Control")
	FVector GetMouseWorldLocation() const;

	void ShowColorWheel();

	void HideColorWheel();

	void UpdateKeyRing();
protected:


	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void SwitchColor(int32 ColorCode);
	void SwitchColorWithArgs(const FInputActionValue& Value, const bool bEnable, const int ColorIndex);


	void FaceMouseCursor();

	void TraceToMouseCursor();

	void CheckNearObject();

	void DrawDetectionConeToMouse();

	void PickupLampeTorche(ALampeTorche* LampeTorche);
	
	void BeginPushOrPull();

	void EndPushOrPull();

	bool CheckForPushableBox();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay();

	void ChangeDebugMode();

	void DropObject();

	void UpdateBatteryUI();

	void PauseGame();

	void Interact();

	void RemapInputsForKeyboardLayout();

	void UpdateInfoBox();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* AzertyMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* QwertyMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TSubclassOf<AActor> DoorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TSubclassOf<AActor> KeyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TSubclassOf<AActor> CurtainClass;
};


// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameOffDevCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "LampeTorche.h"
#include "EnhancedInputSubsystems.h"
#include "Components/TextBlock.h" 
#include "Blueprint/UserWidget.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter)
AGameOffDevCharacter::AGameOffDevCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	SetActorRotation(FRotator(0.0f, -90.0f, 0.0f));
}

void AGameOffDevCharacter::TraceToMouseCursor()
{
	FVector TargetLocation, Direction, Start, End;

	TargetLocation = GetMouseWorldLocation();
	if (TargetLocation != FVector::ZeroVector)
	{
		Start = GetActorLocation();
		Direction = (TargetLocation - Start).GetSafeNormal();
		End = Start + Direction * 1000.f;
		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, -1.0f, 0, 1.0f);

	}
}

FVector AGameOffDevCharacter::GetMouseWorldLocation() const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		float MouseX, MouseY;
		if (PlayerController->GetMousePosition(MouseX, MouseY))
		{
			FVector WorldLocation, WorldDirection;
			if (PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
			{
				FVector Start = WorldLocation;
				FVector End = Start + (WorldDirection * 10000.f);

				FHitResult HitResult;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
				{
					AHighlightableObject* HighlightableObject = Cast<AHighlightableObject>(HitResult.GetActor());
					if (HighlightableObject && !HighlightableObject->getDisplayStatus())
					{
						Params.AddIgnoredActor(HighlightableObject);
					}
					return HitResult.Location;
				}
			}
		}
	}
	return FVector::ZeroVector;
}


void AGameOffDevCharacter::CheckNearObject()
{
	TArray<AActor*> NearbyActors;
	FVector PlayerLocation = GetActorLocation();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALampeTorche::StaticClass(), NearbyActors);
	TArray<AActor*> NearbyBatteries;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattery::StaticClass(), NearbyBatteries);
	NearbyActors.Append(NearbyBatteries);


	for (AActor* Actor : NearbyActors)
	{
		if (Actor->IsHidden())
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
		if (Distance <= 100)
		{
			ABattery* Battery = Cast<ABattery>(Actor);
			if (Battery && CurrentLampeTorche != nullptr)
			{
				if (bIsDebugModeEnabled)
					UE_LOG(LogTemp, Warning, TEXT("Detect Battery"));
				CurrentLampeTorche->Charge(Battery->GetEnergyValue());
				Battery->Destroy();
				break;
			}
			ALampeTorche* LampeTorche = Cast<ALampeTorche>(Actor);
			if (LampeTorche && CurrentLampeTorche == nullptr)
			{
				if (bIsDebugModeEnabled)
					UE_LOG(LogTemp, Warning, TEXT("FlashLight"));
				PickupLampeTorche(LampeTorche);
				break;
			}
		}
	}
}

void AGameOffDevCharacter::PickupLampeTorche(ALampeTorche* LampeTorche)
{
	if (LampeTorche)
	{
		float Distance = FVector::Dist(GetActorLocation(), LampeTorche->GetActorLocation());
		CurrentLampeTorche = LampeTorche;

		USkeletalMeshComponent* PlayerMesh = GetMesh();
		if (PlayerMesh)
		{
			LampeTorche->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("ik_hand_rSocket"));
		}

		LampeTorche->SetActorEnableCollision(false);
	}
}

void AGameOffDevCharacter::FaceMouseCursor()
{
	FVector TargetLocation = GetMouseWorldLocation();

	if (TargetLocation != FVector::ZeroVector)
	{
		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		FRotator NewRotation = Direction.Rotation();
		NewRotation.Pitch = 0.0f;
		NewRotation.Roll = 0.0f;
		NewRotation.Yaw -= 90.0f;

		SetActorRotation(NewRotation);
	}
	if (CurrentLampeTorche)
	{
		FVector SpotDirection = TargetLocation - CurrentLampeTorche->GetActorLocation();
		FRotator SpotRotation = SpotDirection.Rotation();
		CurrentLampeTorche->LampSpotLight->SetWorldRotation(SpotRotation);
	}
}

void AGameOffDevCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FaceMouseCursor();
	CheckNearObject();
	if (bIsDebugModeEnabled == true)
	{
		TraceToMouseCursor();
		DrawDetectionConeToMouse();
	}
	UpdateBatteryUI();
}

void AGameOffDevCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (BatteryWidgetClass)
	{
		BatteryWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), BatteryWidgetClass);
		if (BatteryWidgetInstance)
		{
			BatteryWidgetInstance->AddToViewport();
		}
	}
}

void AGameOffDevCharacter::UpdateBatteryUI()
{
	if (BatteryWidgetInstance)
	{
		UTextBlock* BatteryTextBlock = Cast<UTextBlock>(BatteryWidgetInstance->GetWidgetFromName("BatteryTextBlock"));
		if (BatteryTextBlock)
		{
			if (CurrentLampeTorche != nullptr)
			{
				BatteryTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Battery: %.1f%%"), CurrentLampeTorche->BatteryLevel)));
			}
		}
	}
}

void AGameOffDevCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Look);
		EnhancedInputComponent->BindAction(PushOrPullAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::BeginPushOrPull);
		EnhancedInputComponent->BindAction(PushOrPullAction, ETriggerEvent::Completed, this, &AGameOffDevCharacter::EndPushOrPull);
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputComponent* EnhancedInputComponentColor = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
			{
				EnhancedInputComponentColor->BindAction(SwitchColorAction1, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 1);

				EnhancedInputComponentColor->BindAction(SwitchColorAction2, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 2);

				EnhancedInputComponentColor->BindAction(SwitchColorAction3, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 3);
			}
		}

		EnhancedInputComponent->BindAction(DebugAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::ChangeDebugMode);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::DropObject);
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::Interact);

	}
}


void AGameOffDevCharacter::Interact()
{
	FVector MouseWorldLocation = GetMouseWorldLocation();  // Obtenir la position de la souris dans le monde
	FVector PlayerLocation = GetActorLocation();

	if (MouseWorldLocation != FVector::ZeroVector)
	{
		// Crée une sphere overlap à la position de la souris pour détecter les objets autour
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		// Vérification de l'overlap sur un rayon autour de la souris
		TArray<FHitResult> HitResults;

		// Utilisation d'une sphere pour rechercher des objets autour de la position de la souris
		bool bHit = GetWorld()->SweepMultiByChannel(HitResults, MouseWorldLocation, MouseWorldLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10.f), Params);

		if (bHit)
		{
			for (FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				float Distance = FVector::Dist(PlayerLocation, MouseWorldLocation);
				if (HitActor && HitActor->IsA(DoorClass) && Distance <= 100 && !bIsPushingOrPulling) // Si c'est un BP_Door
				{
					UFunction* OpenDoorFunction = HitActor->FindFunction(TEXT("OpenDoor"));
					if (OpenDoorFunction)
					{
						HitActor->ProcessEvent(OpenDoorFunction, nullptr);
					}
					break;
				}
				if (HitActor->IsA(KeyClass) && Distance <= 100 && !bIsPushingOrPulling)  // Si c'est la clé
				{
					HitActor->Destroy();
				}
				if (HitActor->IsA(CurtainClass) && Distance <= 100 && !bIsPushingOrPulling)  // Si c'est la clé
				{
					UFunction* OpenCurtainFunction = HitActor->FindFunction(TEXT("OpenCurtain"));
					if (OpenCurtainFunction)
					{
						UE_LOG(LogTemp, Warning, TEXT("CURTAIN F"));
						HitActor->ProcessEvent(OpenCurtainFunction, nullptr);
					}
				}
			}
		}
	}
}



void AGameOffDevCharacter::DropObject()
{
	if (CurrentLampeTorche && !bIsPushingOrPulling)
	{
		CurrentLampeTorche->Destroy();
		CurrentLampeTorche = nullptr;
	}
}


void AGameOffDevCharacter::SwitchColorWithArgs(const FInputActionValue& Value, const bool bEnable, const int ColorIndex)
{
	if (bEnable)
	{
		SwitchColor(ColorIndex); 
	}
}


void AGameOffDevCharacter::SwitchColor(int32 ColorCode)
{
	if (CurrentLampeTorche)
	{
		CurrentLampeTorche->ChangeColor(ColorCode);
	}
}


void AGameOffDevCharacter::ChangeDebugMode()
{
	(bIsDebugModeEnabled == true) ? bIsDebugModeEnabled = false : bIsDebugModeEnabled = true;
}

void AGameOffDevCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		APlayerController* PlayerController = Cast<APlayerController>(Controller);
		if (PlayerController)
		{
			FVector CameraDirection = PlayerController->PlayerCameraManager->GetActorForwardVector();
			CameraDirection.Z = 0;
			CameraDirection.Normalize();

			FVector ForwardDirection, RightDirection;
			float Angle = FMath::Atan2(CameraDirection.Y, CameraDirection.X);

			if (Angle >= -PI / 4 && Angle < PI / 4)
			{
				ForwardDirection = FVector(1, 0, 0);
				RightDirection = FVector(0, 1, 0);
			}
			else if (Angle >= PI / 4 && Angle < 3 * PI / 4)
			{
				ForwardDirection = FVector(0, 1, 0);
				RightDirection = FVector(-1, 0, 0);
			}
			else if (Angle >= -3 * PI / 4 && Angle < -PI / 4)
			{
				ForwardDirection = FVector(0, -1, 0);
				RightDirection = FVector(1, 0, 0);
			}
			else
			{
				ForwardDirection = FVector(-1, 0, 0);
				RightDirection = FVector(0, -1, 0);
			}
			if (TargetBox == nullptr)
			{
				AddMovementInput(ForwardDirection, -MovementVector.X);
				AddMovementInput(RightDirection, MovementVector.Y);

			}
			if (bIsPushingOrPulling && TargetBox != nullptr && (MovementVector.Size() > 0) && CheckForPushableBox())
			{
				AddMovementInput(ForwardDirection, -MovementVector.X / 8);
				AddMovementInput(RightDirection, MovementVector.Y / 8);
				FVector PushDirection;

				if (MovementVector.X > 0)
				{
					PushDirection = -ForwardDirection;
				}
				else if (MovementVector.X < 0)
				{
					PushDirection = ForwardDirection;
				}
				else if (MovementVector.Y > 0)
				{
					PushDirection = RightDirection;
				}
				else if (MovementVector.Y < 0)
				{
					PushDirection = -RightDirection;
				}

				FVector NewLocation = TargetBox->GetActorLocation() + (PushDirection / 2); // Ajustez la vitesse ici
				TargetBox->SetActorLocation(NewLocation);
			}
		}
	}
}

void AGameOffDevCharacter::EndPushOrPull()
{
	bIsPushingOrPulling = false;
	TargetBox = nullptr;

	if (CurrentLampeTorche)
	{
		CurrentLampeTorche->LampSpotLight->SetVisibility(true);
	}
}

bool AGameOffDevCharacter::CheckForPushableBox()
{
	FVector Start = GetActorLocation();

	FVector ForwardVector = GetActorForwardVector();
	FRotator NewRotation = FRotator(0.f, 90.f, 0.f);
	ForwardVector = NewRotation.RotateVector(ForwardVector);
	FVector End = Start + ForwardVector * 50.0f;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		APoussableBox* DetectedBox = Cast<APoussableBox>(HitResult.GetActor());
		if (DetectedBox)
			return true;
	}
	return false;
}

void AGameOffDevCharacter::BeginPushOrPull()
{
	FVector Start = GetActorLocation();

	FVector ForwardVector = GetActorForwardVector();
	FRotator NewRotation = FRotator(0.f, 90.f, 0.f);
	ForwardVector = NewRotation.RotateVector(ForwardVector);

	FVector End = Start + ForwardVector * 50.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{


		if (bIsDebugModeEnabled)
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
		TargetBox = Cast<APoussableBox>(HitResult.GetActor());
		if (TargetBox != nullptr)
		{
			bIsPushingOrPulling = true;
			if (CurrentLampeTorche)
			{
				CurrentLampeTorche->LampSpotLight->SetVisibility(false);
			}
		}
	}
	else
	{
		if (bIsDebugModeEnabled)
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);
	}
}

void AGameOffDevCharacter::DrawDetectionConeToMouse()
{
	if (!CurrentLampeTorche || !CurrentLampeTorche->LampSpotLight)
	{
		return;
	}

	FVector HandPosition = GetMesh()->GetSocketLocation(TEXT("ik_hand_rSocket"));
	FVector MouseWorldPosition = GetMouseWorldLocation();

	FVector MouseWorldDirection;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	MouseWorldDirection = (MouseWorldPosition - CameraLocation).GetSafeNormal();
	FVector CharacterDirection = (MouseWorldPosition - HandPosition).GetSafeNormal();

	float Length = CurrentLampeTorche->LampSpotLight->AttenuationRadius;
	float ConeAngle = CurrentLampeTorche->LampSpotLight->OuterConeAngle;

	DrawDebugCone(
		GetWorld(),
		HandPosition,
		CharacterDirection,
		Length,
		FMath::DegreesToRadians(ConeAngle),
		FMath::DegreesToRadians(ConeAngle),
		12,
		FColor::Green,
		false,
		-1.0f,
		0,
		1.0f
	);
}

void AGameOffDevCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
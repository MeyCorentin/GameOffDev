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
#include "EnhancedInputSubsystems.h"
#include "LampeTorche.h"
#include "ColorFilter.h"
#include <Windows.h>
#include "EnhancedInputSubsystems.h"
#include "Components/ProgressBar.h"
#include "GameOffDevGameMode.h"

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
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.0f, 0, 1.0f);

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
				Params.AddIgnoredActor(this); // Ignorer le joueur lui-même

				bool bFirstHit = true;

				while (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
				{
					AActor* HitActor = HitResult.GetActor();
					if (bFirstHit)
					{
						Params.AddIgnoredActor(HitActor);
						bFirstHit = false;
					}
					else
					{
						AHighlightableObject* HighlightableObject = Cast<AHighlightableObject>(HitActor);
						if (HighlightableObject && !HighlightableObject->getDisplayStatus())
						{
							Params.AddIgnoredActor(HighlightableObject);
						}

						return HitResult.Location;
					}
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
			LampeTorche->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("RightHandIndex4Socket"));
			LampeTorche->SetActorScale3D(LampeTorche->GetActorScale3D() * 3.0f);
		}

		LampeTorche->SetActorEnableCollision(false);
	}
}

void AGameOffDevCharacter::RemapInputsForKeyboardLayout()
{
	// Récupère le layout de clavier actif
	HKL KeyboardLayout = GetKeyboardLayout(0);
	LANGID LangID = LOWORD(KeyboardLayout);

	// Détecte si le clavier est AZERTY (French)
	bool bIsAzerty = (LangID == MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH));

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem) return;

	InputSubsystem->ClearAllMappings();

	if (bIsAzerty)
	{
		InputSubsystem->AddMappingContext(AzertyMappingContext, 0);
	}
	else
	{
		InputSubsystem->AddMappingContext(QwertyMappingContext, 0);
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
	UpdateInfoBox();
	UpdateKeyRing();
}

void AGameOffDevCharacter::BeginPlay()
{
	Super::BeginPlay();
	RemapInputsForKeyboardLayout();
	if (BatteryWidgetClass)
	{
		BatteryWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), BatteryWidgetClass);
		if (BatteryWidgetInstance)
		{
			BatteryWidgetInstance->AddToViewport();
		}
	}
	if (ColorWheelWidgetClass)
	{
		ColorWheelWidget = CreateWidget<UUserWidget>(GetWorld(), ColorWheelWidgetClass);
	}
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();
	KeyRing = GetWorld()->SpawnActor<AActor>(MyKeyRingBlueprint, SpawnLocation, SpawnRotation);
	USkeletalMeshComponent* PlayerMesh = GetMesh();

	if (PlayerMesh)
	{
		KeyRing->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("LeftHandIndex4Socket"));
		KeyRing->SetActorEnableCollision(false);
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		KeyRing->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
		{
			MeshComponent->SetVisibility(false);
		}
	}
	if (AGameOffDevGameMode* GameMode = Cast<AGameOffDevGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

		if (!CurrentLevelName.Equals(TEXT("Level1"), ESearchCase::IgnoreCase))
		{
			ALampeTorche* NewLampeTorche = GetWorld()->SpawnActor<ALampeTorche>(ALampeTorche::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			PickupLampeTorche(NewLampeTorche);
		}
	}
}

void AGameOffDevCharacter::UpdateBatteryUI()
{
	if (BatteryWidgetInstance)
	{
		// Obtenez la ProgressBar à partir du Widget
		UProgressBar* BatteryProgressBar = Cast<UProgressBar>(BatteryWidgetInstance->GetWidgetFromName("BatteryBar"));
		if (BatteryProgressBar)
		{
			if (CurrentLampeTorche != nullptr)
			{
				// Met à jour la valeur de la ProgressBar en fonction du niveau de batterie
				// Assurez-vous que BatteryLevel est un float entre 0.0 et 1.0 pour le bon fonctionnement de la barre
				BatteryProgressBar->SetPercent(FMath::Clamp(CurrentLampeTorche->BatteryLevel / 100.0f, 0.0f, 1.0f));
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

		EnhancedInputComponent->BindAction(WheelAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::ShowColorWheel);
		EnhancedInputComponent->BindAction(WheelAction, ETriggerEvent::Completed, this, &AGameOffDevCharacter::HideColorWheel);
		EnhancedInputComponent->BindAction(WheelAction, ETriggerEvent::Canceled, this, &AGameOffDevCharacter::HideColorWheel);

	}
}

void AGameOffDevCharacter::ShowColorWheel()
{
	if (ColorWheelWidget && !ColorWheelWidget->IsInViewport())
	{
		ColorWheelWidget->AddToViewport();
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			PlayerController->bShowMouseCursor = true;
		}
	}
}

void AGameOffDevCharacter::HideColorWheel()
{
	if (ColorWheelWidget && ColorWheelWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hide color wheel"));
		ColorWheelWidget->RemoveFromViewport();
		ColorWheelWidget->RemoveFromParent();
	}
}


void AGameOffDevCharacter::UpdateInfoBox()
{
	FVector MouseWorldLocation = GetMouseWorldLocation();
	FVector PlayerLocation = GetActorLocation();
	UTextBlock* InfoBox = Cast<UTextBlock>(BatteryWidgetInstance->GetWidgetFromName("InfoBox"));

	if (MouseWorldLocation != FVector::ZeroVector)
	{
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		TArray<FHitResult> HitResults;
		bool bHit = GetWorld()->SweepMultiByChannel(HitResults, MouseWorldLocation, MouseWorldLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10.f), Params);

		InfoBox->SetText(FText::FromString(FString::Printf(TEXT(""))));
		if (bHit)
		{
			for (FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				float Distance = FVector::Dist(PlayerLocation, MouseWorldLocation);
				if (HitActor && Distance <= 100)
				{
					if (HitActor->IsA(DoorClass))
					{
						FText DisplayText;
						FName VariableName(TEXT("DisplayText"));

						FProperty* Property = HitActor->GetClass()->FindPropertyByName(VariableName);
						if (Property && Property->IsA(FTextProperty::StaticClass()))
						{
							FTextProperty* TextProperty = CastField<FTextProperty>(Property);
							if (TextProperty)
							{
								DisplayText = TextProperty->GetPropertyValue_InContainer(HitActor);
								InfoBox->SetText(DisplayText);
							}
						}
					}
					if (HitActor->IsA(KeyClass))
					{
						FText DisplayText;
						FName VariableName(TEXT("DisplayText"));

						FProperty* Property = HitActor->GetClass()->FindPropertyByName(VariableName);
						if (Property && Property->IsA(FTextProperty::StaticClass()))
						{
							FTextProperty* TextProperty = CastField<FTextProperty>(Property);
							if (TextProperty)
							{
								DisplayText = TextProperty->GetPropertyValue_InContainer(HitActor);
								InfoBox->SetText(DisplayText);
							}
						}
					}
					if (HitActor->IsA(CurtainClass))
					{
						FText DisplayText;
						FName VariableName(TEXT("DisplayText"));

						FProperty* Property = HitActor->GetClass()->FindPropertyByName(VariableName);
						if (Property && Property->IsA(FTextProperty::StaticClass()))
						{
							FTextProperty* TextProperty = CastField<FTextProperty>(Property);
							if (TextProperty)
							{
								DisplayText = TextProperty->GetPropertyValue_InContainer(HitActor);
								InfoBox->SetText(DisplayText);
							}
						}
					}
					if (HitActor->IsA(AColorFilter::StaticClass()))
					{
						AColorFilter* ColorFilterActor = Cast<AColorFilter>(HitActor);
						if (ColorFilterActor)
						{
							FText DisplayText = ColorFilterActor->GetDisplayText();
							InfoBox->SetText(DisplayText);
						}
					}
				}
			}
		}
	}
}




void AGameOffDevCharacter::Interact()
{
	FVector MouseWorldLocation = GetMouseWorldLocation();
	FVector PlayerLocation = GetActorLocation();

	if (MouseWorldLocation != FVector::ZeroVector)
	{
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		TArray<FHitResult> HitResults;
		bool bHit = GetWorld()->SweepMultiByChannel(HitResults, MouseWorldLocation, MouseWorldLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10.f), Params);

		if (bHit)
		{
			for (FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				float Distance = FVector::Dist(PlayerLocation, MouseWorldLocation);
				if (HitActor && HitActor->IsA(DoorClass) && Distance <= 100 && !bIsPushingOrPulling)
				{
					UFunction* OpenDoorFunction = HitActor->FindFunction(TEXT("OpenDoor"));
					if (OpenDoorFunction)
					{
						HitActor->ProcessEvent(OpenDoorFunction, nullptr);
					}
					break;
				}
				if (HitActor->IsA(KeyClass) && Distance <= 100 && !bIsPushingOrPulling)
				{
					HitActor->Destroy();
				}
				if (HitActor->IsA(CurtainClass) && Distance <= 100 && !bIsPushingOrPulling)  // Si c'est la clé
				{
					UFunction* OpenCurtainFunction = HitActor->FindFunction(TEXT("OpenCurtain"));
					if (OpenCurtainFunction)
					{
						HitActor->ProcessEvent(OpenCurtainFunction, nullptr);
					}
				}
				if (HitActor->IsA(AColorFilter::StaticClass()) && Distance <= 100 && !bIsPushingOrPulling)
				{
					AColorFilter* Filter = Cast<AColorFilter>(HitActor);
					if (Filter && CurrentLampeTorche != nullptr)
					{
						int32 FilterValue = Filter->Value;
						if (FilterValue >= 0 && FilterValue < CurrentLampeTorche->_ColorFilter.Num())
							CurrentLampeTorche->PicktupColor(FilterValue);
						Filter->Destroy();
					}
				}
			}
		}
	}
}
void AGameOffDevCharacter::UpdateKeyRing()
{
	TArray<AActor*> DoorActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), DoorClass, DoorActors);
	int temp_key_number = 0;
	for (AActor* HitActor : DoorActors)
	{
		if (HitActor->IsA(DoorClass))
		{
			TArray<UStaticMeshComponent*> StaticMeshComponents;
			HitActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

			UStaticMeshComponent* LockComponent = nullptr;

			for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
			{
				if (MeshComponent && MeshComponent->GetName() == TEXT("Lock"))
					LockComponent = MeshComponent;
			}


			FName VariableName(TEXT("RequiredKey"));
			FProperty* Property = HitActor->GetClass()->FindPropertyByName(VariableName);
			bool keyIsValid = true;

			if (Property)
			{
				if (Property->IsA(FObjectProperty::StaticClass()))
				{
					FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
					if (ObjectProperty)
					{
						AActor*RequiredKeyActor = Cast<AActor>(ObjectProperty->GetPropertyValue_InContainer(HitActor));
						keyIsValid = IsValid(RequiredKeyActor);
					}
				}
			}

			if (LockComponent)
			{
				if (LockComponent->IsVisible() && !keyIsValid)
				{
					temp_key_number++;
				}
			}
		}
	}
	if (temp_key_number != key_number)
	{
		key_number = temp_key_number;

		if (MyKeyRingBlueprint)
		{
			TArray<UStaticMeshComponent*> StaticMeshComponents;
			KeyRing->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

			UE_LOG(LogTemp, Warning, TEXT("KEY NUMBER : %i"), key_number);
			for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
			{
				MeshComponent->SetVisibility(false);

				if (key_number == 1 && MeshComponent->GetName() == TEXT("1Key"))
				{
					MeshComponent->SetVisibility(true);
				}
				if (key_number == 2 && MeshComponent->GetName() == TEXT("2Key"))
				{
					MeshComponent->SetVisibility(true);
				}
				if (key_number == 3 && MeshComponent->GetName() == TEXT("3Key"))
				{
					MeshComponent->SetVisibility(true);
				}
				if (key_number == 0 && MeshComponent->GetName() == TEXT("0Key"))
				{
					MeshComponent->SetVisibility(true);
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
		UE_LOG(LogTemp, Warning, TEXT("Color %i"), ColorIndex);
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

	/*DrawDebugCone(
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
	);*/
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
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
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h" 
#include "Blueprint/UserWidget.h"
#include "InputActionValue.h"
#include "GameOffGameInstance.h"

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
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.0f, 0, 1.0f);

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
				CurrentLampeTorche->Charge(Battery->GetEnergyValue());
				Battery->Destroy();
				break;
			}
			ALampeTorche* LampeTorche = Cast<ALampeTorche>(Actor);
			if (LampeTorche && CurrentLampeTorche == nullptr)
			{
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
		BatteryWidgetInstance->AddToViewport();
	}
}

void AGameOffDevCharacter::RemapInputsForKeyboardLayout()
{
	HKL KeyboardLayout = GetKeyboardLayout(0);
	LANGID LangID = LOWORD(KeyboardLayout);

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

void AGameOffDevCharacter::FaceCameraToCharacter()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		AActor* CameraActor = PlayerController->GetViewTarget();
		if (CameraActor)
		{
			FVector CameraLocation = CameraActor->GetActorLocation();
			FVector CharacterLocation = GetActorLocation();
			FVector DirectionToCharacter = CharacterLocation - CameraLocation;
			DirectionToCharacter.Z = 0;
			FRotator NewRotation = DirectionToCharacter.Rotation();
			CameraActor->SetActorRotation(NewRotation);
		}
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
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (CurrentLevelName.Equals(TEXT("Level2"), ESearchCase::IgnoreCase))
	{
		//FaceCameraToCharacter();
	}
	if (!display_wheel)
	{

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

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController && CurrentLampeTorche != nullptr)
	{
		if (PlayerController->IsInputKeyDown(EKeys::RightMouseButton))
		{
			UE_LOG(LogTemp, Warning, TEXT("RIGHT CLICk"));
			if (!display_wheel)
			{
				ShowColorWheel();
			}	
			if (display_wheel)
			{
				FVector2D InputDirection = FVector2D(0, 0);
				FString ImageName = "C" + FString::FromInt(CurrentLampeTorche->DefaultColor);
				ShowColorImage(ImageName);
				if (PlayerController->IsInputKeyDown(EKeys::Z) || PlayerController->IsInputKeyDown(EKeys::Up))
				{
					SwitchColor(0);
					ShowColorImage("C0");
				}
				else if (PlayerController->IsInputKeyDown(EKeys::S) || PlayerController->IsInputKeyDown(EKeys::Down))
				{
					SwitchColor(1);
					ShowColorImage("C1");
				}
				if (PlayerController->IsInputKeyDown(EKeys::Q) || PlayerController->IsInputKeyDown(EKeys::Left))
				{
					SwitchColor(2);
					ShowColorImage("C2");
				}
				else if (PlayerController->IsInputKeyDown(EKeys::D) || PlayerController->IsInputKeyDown(EKeys::Right))
				{
					SwitchColor(3);
					ShowColorImage("C3");
				}
			}
		}
		else
		{
			if (display_wheel) 
			{
				HideColorWheel();
			}
		}
	}
}

void AGameOffDevCharacter::ShowColorImage(const FString& ImageName)
{
	if (ColorWheelWidget)
	{
		if (UWidget* RootWidget = ColorWheelWidget->GetRootWidget())
		{
			if (UCanvasPanel* CanvasPanel = Cast<UCanvasPanel>(RootWidget))
			{
				for (int32 i = 0; i < CanvasPanel->GetChildrenCount(); ++i)
				{
					UWidget* ChildWidget = CanvasPanel->GetChildAt(i);
					if (UImage* Image = Cast<UImage>(ChildWidget))
					{
						Image->SetVisibility(ESlateVisibility::Collapsed);
						if (Image->GetName() == ImageName)
						{
							Image->SetVisibility(ESlateVisibility::Visible);
						}
					}
				}
			}
		}
	}
}

void AGameOffDevCharacter::ShowColorWheel()
{

	if (ColorWheelWidget->Visibility != ESlateVisibility::Visible)
	{
		ColorWheelWidget->SetVisibility(ESlateVisibility::Visible);
		display_wheel = true;

		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			PlayerController->bShowMouseCursor = true;
		}
	}
}

void AGameOffDevCharacter::HideColorWheel()
{
	if (!ColorWheelWidget)
	{
		return;
	}

	if (IsValid(ColorWheelWidget) && ColorWheelWidget->Visibility != ESlateVisibility::Collapsed)
	{
		ColorWheelWidget->SetVisibility(ESlateVisibility::Collapsed);
		display_wheel = false;
	}
}


void AGameOffDevCharacter::BeginPlay()
{
	Super::BeginPlay();
	RemapInputsForKeyboardLayout();
	if (BatteryWidgetClass)
	{
		BatteryWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), BatteryWidgetClass);
	}
	if (ColorWheelWidgetClass)
	{
		ColorWheelWidget = CreateWidget<UUserWidget>(GetWorld(), ColorWheelWidgetClass);
		if (ColorWheelWidget)
		{
			ColorWheelWidget->AddToViewport();
			ColorWheelWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
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
			NewLampeTorche->DefaultColor = 0;
			PickupLampeTorche(NewLampeTorche);
		}
	}
	LoadPlayerData();
}

void AGameOffDevCharacter::UpdateBatteryUI()
{
	if (CurrentLampeTorche != nullptr)
	{
		if (BatteryWidgetInstance != nullptr)
		{
			UProgressBar* BatteryProgressBar = Cast<UProgressBar>(BatteryWidgetInstance->GetWidgetFromName("BatteryBar"));
			if (BatteryProgressBar)
			{
				if (CurrentLampeTorche != nullptr)
				{
					BatteryProgressBar->SetPercent(FMath::Clamp(CurrentLampeTorche->BatteryLevel / 100.0f, 0.0f, 1.0f));
				}
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Look);
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputComponent* EnhancedInputComponentColor = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
			{
				EnhancedInputComponentColor->BindAction(SwitchColorAction1, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 0);

				EnhancedInputComponentColor->BindAction(SwitchColorAction2, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 1);

				EnhancedInputComponentColor->BindAction(SwitchColorAction3, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 2);

				EnhancedInputComponentColor->BindAction(SwitchColorAction4, ETriggerEvent::Started, this,
					&AGameOffDevCharacter::SwitchColorWithArgs, true, 3);
			}
		}

		EnhancedInputComponent->BindAction(DebugAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::ChangeDebugMode);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::DropObject);
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &AGameOffDevCharacter::Interact);
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Completed, this, &AGameOffDevCharacter::EndPushOrPull);
	}
}

void AGameOffDevCharacter::Jump()
{
	if (CanJumpBasedOnBox())
	{
		Super::Jump();
	}
}


bool AGameOffDevCharacter::CanJumpBasedOnBox()
{
	FVector PlayerLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	FVector SphereCenter = PlayerLocation + ForwardVector * 30.0f;
	SphereCenter.Z -= 70.0f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		SphereCenter,
		SphereCenter,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(50.0f),
		Params
	);

	if (bHit)
	{
		for (FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->IsA(APoussableBox::StaticClass()) && !bIsPushingOrPulling)
			{
				//DrawDebugSphere(GetWorld(), SphereCenter, 50.0f, 12, FColor::Green, false, 2.0f);  // Affiche un cercle vert

				return true;
			}
		}
	}

	return false;
}


void AGameOffDevCharacter::DisableGameplayInputs()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->ClearBindingValues();
	}
}


void AGameOffDevCharacter::EnableGameplayInputs()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Look);
	}
}

void AGameOffDevCharacter::UpdateInfoBox()
{
	FVector MouseWorldLocation = GetMouseWorldLocation();
	FVector PlayerLocation = GetActorLocation();
	if (BatteryWidgetInstance != nullptr)
	{
		UTextBlock* InfoBox = Cast<UTextBlock>(BatteryWidgetInstance->GetWidgetFromName("InfoBox"));

		if (!InfoBox)
		{
			return;
		}
		if (MouseWorldLocation != FVector::ZeroVector)
		{
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			TArray<FHitResult> HitResults;
			bool bHit = GetWorld()->SweepMultiByChannel(HitResults, MouseWorldLocation, MouseWorldLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10.f), Params);
			if  (InfoBox != nullptr)
			{
				InfoBox->SetText(FText::FromString(FString::Printf(TEXT(""))));
				InfoBox->SetText(FText::FromString(FString::Printf(TEXT(""))));
			}
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
						if (HitActor->IsA(PaperClass))
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
}

void AGameOffDevCharacter::Interact()
{
	FVector MouseWorldLocation = GetMouseWorldLocation();
	FVector PlayerLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();

	FVector BoxSphereCenter = PlayerLocation + ForwardVector * 30.0f;  // Position devant le joueur
	BoxSphereCenter.Z -= 50.0f;
	FCollisionQueryParams BoxParams;
	BoxParams.AddIgnoredActor(this);
	TArray<FHitResult> BoxHitResults;

	bool bBoxHit = GetWorld()->SweepMultiByChannel(BoxHitResults, BoxSphereCenter, BoxSphereCenter, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(50.0f), BoxParams);

	//DrawDebugSphere(GetWorld(), BoxSphereCenter, 50.0f, 12, FColor::Green, false, 2.0f);

	if (bBoxHit)
	{
		for (FHitResult& BoxHit : BoxHitResults)
		{
			AActor* BoxHitActor = BoxHit.GetActor();
			if (BoxHitActor && BoxHitActor->IsA(APoussableBox::StaticClass()) && !bIsPushingOrPulling)
			{
				BeginPushOrPull();
				break;
			}
		}
	}
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
				if (HitActor->IsA(PaperClass) && Distance <= 100 && !bIsPushingOrPulling)
				{
					CanEscape = true;
					HitActor->Destroy();
				}
				if (HitActor->IsA(CurtainClass) && Distance <= 100 && !bIsPushingOrPulling)
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
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController->IsInputKeyDown(EKeys::RightMouseButton))
	{
		return;
	}
	if (Controller != nullptr && !display_wheel && !PlayerController->IsInputKeyDown(EKeys::RightMouseButton))
	{
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
			if (bIsPushingOrPulling && TargetBox != nullptr && (MovementVector.Size() > 0))
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

				FVector NewLocation = TargetBox->GetActorLocation() + (PushDirection / 2);
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

void AGameOffDevCharacter::BeginPushOrPull()
{
	FVector Start = GetActorLocation();

	// Récupérer la position de la souris dans le monde
	FVector End = GetMouseWorldLocation();

	if (End.IsZero())
	{
		return;
	}
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
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	MouseWorldDirection = (MouseWorldPosition - CameraLocation).GetSafeNormal();
	FVector CharacterDirection = (MouseWorldPosition - HandPosition).GetSafeNormal();

	float Length = CurrentLampeTorche->LampSpotLight->AttenuationRadius;
	float ConeAngle = CurrentLampeTorche->LampSpotLight->OuterConeAngle;
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

void AGameOffDevCharacter::SavePlayerData()
{
	UGameOffGameInstance* GameInstance = Cast<UGameOffGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GameInstance->CurrentSave)
	{
		GameInstance->CurrentSave = Cast<UGameOffSaveGame>(UGameplayStatics::CreateSaveGameObject(UGameOffSaveGame::StaticClass()));
	}

	if (GameInstance && GameInstance->CurrentSave)
	{
		if (CurrentLampeTorche && CurrentLampeTorche->_ColorFilter.Num() > 0)
		{
			GameInstance->CurrentSave->FilterInventory = CurrentLampeTorche->_ColorFilter;
			FString FilterInventoryLog;
			for (bool bValue : GameInstance->CurrentSave->FilterInventory)
			{
				FilterInventoryLog += bValue ? TEXT("true, ") : TEXT("false, ");
			}
		}
		GameInstance->CurrentSave->FilterInventory = CurrentLampeTorche->_ColorFilter;
		GameInstance->CurrentSave->PreviousLevelName = UGameplayStatics::GetCurrentLevelName(this);
		GameInstance->CurrentSave->DefaultColor = CurrentLampeTorche->DefaultColor;
		GameInstance->CurrentSave->LightColor = CurrentLampeTorche->LampSpotLight->GetLightColor();
		GameInstance->CurrentSave->BatteryLevel = CurrentLampeTorche->BatteryLevel;
		GameInstance->CurrentSave->BatteryDrainRate = CurrentLampeTorche->BatteryDrainRate;
		GameInstance->CurrentSave->LightRadiusFactor = CurrentLampeTorche->LightRadiusFactor;
		GameInstance->CurrentSave->LightIntensityFactor = CurrentLampeTorche->LightIntensityFactor;
		GameInstance->CurrentSave->InitBatteryLevel = CurrentLampeTorche->InitBatteryLevel;
		GameInstance->CurrentSave->InitialIntensity = CurrentLampeTorche->InitialIntensity;
		GameInstance->CurrentSave->InitialAttenuationRadius = CurrentLampeTorche->InitialAttenuationRadius;

		GameInstance->CurrentSave->InnerConeAngle = CurrentLampeTorche->LampSpotLight->InnerConeAngle;
		if (GameInstance->CurrentSave->CanEscape == false)
			GameInstance->CurrentSave->CanEscape = CanEscape;
		GameInstance->SaveGameData();
	}
}

void AGameOffDevCharacter::LoadPlayerData()
{

	UGameOffGameInstance* GameInstance = Cast<UGameOffGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GameInstance && GameInstance->CurrentSave)
	{
		FVector SpawnLocation;

		if (GameInstance->CurrentSave->PreviousLevelName == "Level1")
		{
			SpawnLocation = FVector(272.071216f, 0.000000f, 103.500847f);
		}
		else if (GameInstance->CurrentSave->PreviousLevelName == "Level3")
		{
			SpawnLocation = FVector(-367.928784f, 20.000000f, 103.500847f);
		}
		else if (GameInstance->CurrentSave->PreviousLevelName == "Level4")
		{
			SpawnLocation = FVector(412.071216, 40.0, 113.500847f);
		}
		else if (GameInstance->CurrentSave->PreviousLevelName == "Level5")
		{
			SpawnLocation = FVector(-347.928784f, -310.000000f, 313.500847f);
		}
		else
		{
			SpawnLocation = GetActorLocation();
		}
		SetActorLocation(SpawnLocation);
		if (CurrentLampeTorche && GameInstance->CurrentSave->FilterInventory.Num() > 0)
		{
			CurrentLampeTorche->_ColorFilter = GameInstance->CurrentSave->FilterInventory;
		}
		CurrentLampeTorche->DefaultColor = GameInstance->CurrentSave->DefaultColor;
		CurrentLampeTorche->_ColorFilter = GameInstance->CurrentSave->FilterInventory;
		CurrentLampeTorche->LampSpotLight->SetLightColor(GameInstance->CurrentSave->LightColor);
		CurrentLampeTorche->BatteryLevel = GameInstance->CurrentSave->BatteryLevel;
		CurrentLampeTorche->BatteryDrainRate = GameInstance->CurrentSave->BatteryDrainRate;
		CurrentLampeTorche->LightRadiusFactor = GameInstance->CurrentSave->LightRadiusFactor;
		CurrentLampeTorche->LightIntensityFactor = GameInstance->CurrentSave->LightIntensityFactor;
		CurrentLampeTorche->InitBatteryLevel = 100;
		CurrentLampeTorche->InitialIntensity = 400000;
		CanEscape = GameInstance->CurrentSave->CanEscape;
		CurrentLampeTorche->LampSpotLight->InnerConeAngle = GameInstance->CurrentSave->InnerConeAngle;
		CurrentLampeTorche->InitialAttenuationRadius = GameInstance->CurrentSave->InitialAttenuationRadius;
	}

}
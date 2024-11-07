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
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter)
AGameOffDevCharacter::AGameOffDevCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
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
	FVector TargetLocation = GetMouseWorldLocation();

	if (TargetLocation != FVector::ZeroVector)
	{
		FVector Start = GetActorLocation();

		FVector Direction = (TargetLocation - Start).GetSafeNormal();

		FVector End = Start + Direction * 1000.f;

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);


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
				// Tracer une ligne pour obtenir le point d'impact
				FVector Start = WorldLocation;
				FVector End = Start + (WorldDirection * 10000.f); // Distance de projection

				FHitResult HitResult;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
				{
					return HitResult.Location;
				}
			}
		}
	}
	return FVector::ZeroVector; // Si aucun impact, retourne un vecteur nul
}


void AGameOffDevCharacter::CheckForLampeTorche()
{
	TArray<AActor*> NearbyActors;
	FVector PlayerLocation = GetActorLocation();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALampeTorche::StaticClass(), NearbyActors);

	for (AActor* Actor : NearbyActors)
	{
		ALampeTorche* LampeTorche = Cast<ALampeTorche>(Actor);
		if (LampeTorche)
		{
			float Distance = FVector::Dist(PlayerLocation, LampeTorche->GetActorLocation());
			if (Distance <= 200.0f)
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
			LampeTorche->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("ik_hand_rSocket"));
		}

		LampeTorche->SetActorEnableCollision(false);
	}
}




void AGameOffDevCharacter::TraceForward()
{
    FVector Start = GetActorLocation();

    FVector ForwardVector = GetActorForwardVector();
	FRotator NewRotation = FRotator(0.f, 90.f, 0.f);
	ForwardVector = NewRotation.RotateVector(ForwardVector);

    FVector End = Start + ForwardVector * 50.0f;

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

    FVector ImpactLocation = HitResult.Location;

    if (bHit)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, -1.0f, 0, 1.0f);
    }
    else
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.0f, 0, 1.0f);
    }
}



void AGameOffDevCharacter::FaceMouseCursor()
{
	FVector TargetLocation = GetMouseWorldLocation();

	if (TargetLocation != FVector::ZeroVector)
	{
		// Calcul de la direction vers la souris
		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		FRotator NewRotation = Direction.Rotation();
		NewRotation.Pitch = 0.0f; // Conserver seulement la rotation sur l'axe Yaw
		NewRotation.Roll = 0.0f;
		NewRotation.Yaw -=90.0f;

		// Applique la rotation à l'ensemble du personnage, y compris la capsule et le mesh
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
	FaceMouseCursor(); // Met à jour la rotation du personnage
	TraceForward();
	TraceToMouseCursor();
	CheckForLampeTorche();
}

void AGameOffDevCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGameOffDevCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGameOffDevCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}
void AGameOffDevCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		APlayerController* PlayerController = Cast<APlayerController>(Controller);
		if (PlayerController)
		{
			FVector CameraDirection = PlayerController->PlayerCameraManager->GetActorForwardVector();

			FVector ForwardDirection = CameraDirection;
			FVector RightDirection = FVector::CrossProduct(FVector::UpVector, CameraDirection); 
			ForwardDirection.Normalize();
			RightDirection.Normalize();

			AddMovementInput(RightDirection, MovementVector.Y);
			AddMovementInput(ForwardDirection, -MovementVector.X);
		}
	}
}


void AGameOffDevCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
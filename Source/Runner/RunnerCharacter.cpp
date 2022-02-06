// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red,text)

//////////////////////////////////////////////////////////////////////////
// ARunnerCharacter

ARunnerCharacter::ARunnerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create gun mesh
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	GunMeshComponent->SetupAttachment(GetMesh(), WeaponSocketName);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARunnerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARunnerCharacter::StartFire);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ARunnerCharacter::SlideStarted);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARunnerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARunnerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("JumpOrCrouch", this, &ARunnerCharacter::JumpOrCrouchAxis);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	/*
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARunnerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARunnerCharacter::LookUpAtRate);
	*/

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARunnerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARunnerCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARunnerCharacter::OnResetVR);
}

void ARunnerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MoveForward(1.0);
}

void ARunnerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GunMeshComponent->AttachTo(GetMesh(), WeaponSocketName, EAttachLocation::SnapToTarget, false);
}


void ARunnerCharacter::OnResetVR()
{
	// If Runner is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in Runner.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARunnerCharacter::JumpOrCrouchAxis(float Value)
{
	if (Value > .8f)
	{
		if (GetCharacterMovement()->IsFalling())
		{
			return;
		}
		Jump();
		return;
	}
	if (Value < -.8f)
	{
		SlideStarted();
		return;
	}
}

void ARunnerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	TouchDownLoc = Location;
}

void ARunnerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	FVector tempWorldLocation;
	FVector tempWorldDirection;
	UGameplayStatics::GetPlayerController(this, 0)->DeprojectScreenPositionToWorld(Location.X, Location.Y, tempWorldLocation, tempWorldDirection);
	Fire(SetAim(tempWorldLocation, tempWorldDirection));
}

FVector ARunnerCharacter::SetAim(FVector worldLocation, FVector worldDirection)
{
	FVector muzzleLoc = GunMeshComponent->GetSocketLocation(MuzzleSocketName);
	FVector end = worldLocation + worldDirection * 4000;
	FRotator weaponRotation = UKismetMathLibrary::FindLookAtRotation(muzzleLoc, end);
	Pitch = weaponRotation.Pitch;
	Yaw = weaponRotation.Yaw;
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, worldLocation, end, ECollisionChannel::ECC_Camera, CollisionQueryParams))
	{
		return HitResult.Location;
	}

	return end;
}

void ARunnerCharacter::StartFire()
{
	FVector tempWorldLocation;
	FVector tempWorldDirection;
	UGameplayStatics::GetPlayerController(this, 0)->DeprojectMousePositionToWorld(tempWorldLocation, tempWorldDirection);
	Fire(SetAim(tempWorldLocation, tempWorldDirection));
}

void ARunnerCharacter::Fire(FVector aimLoc)
{
	if (GetCharacterMovement()->IsCrouching())
	{
		return;
	}
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}
	if (Pitch > 80 || Yaw > 80)
	{
		return;
	}
	FVector muzzleLoc = GunMeshComponent->GetSocketLocation(MuzzleSocketName);
	FRotator prjRot = UKismetMathLibrary::FindLookAtRotation(muzzleLoc, aimLoc);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(Projectile, muzzleLoc, prjRot, SpawnParams);
}

void ARunnerCharacter::SlideStarted()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		return;
	}
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}
	UAnimInstance* AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance == nullptr)
	{
		return;
	}
	Crouch();
	AnimInstance->Montage_Play(SlideMontage, 1.0f);
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ARunnerCharacter::SlideEnded);
}

void ARunnerCharacter::SlideEnded(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	UnCrouch();
}

void ARunnerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARunnerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARunnerCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value >= 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARunnerCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

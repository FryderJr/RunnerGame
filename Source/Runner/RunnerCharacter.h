// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RunnerCharacter.generated.h"

UCLASS(config=Game)
class ARunnerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	class USkeletalMeshComponent* GunMeshComponent;

	FVector TouchDownLoc;

public:
	ARunnerCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	float Pitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	float Yaw;

	bool bIsSliding;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement)
	bool bCanTurn;

	FRotator DesiredRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName WeaponSocketName;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<AActor> Projectile;

	UPROPERTY(EditDefaultsOnly, Category = Control)
	class UAnimMontage* SlideMontage;

	UPROPERTY(EditDefaultsOnly, Category = Shield)
	float ShieldTime = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Shield)
	bool bIsShielded;

	class UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Control)
	TArray<FVector> LanesPositions;

	UPROPERTY(EditDefaultsOnly, Category = Control)
	int CurrentLane;

	int TargetLane;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight();

	void MoveLeft();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** */
	void JumpOrCrouchAxis(float Value);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UFUNCTION(BlueprintCallable, Category = Control)
	void ChangeLanes(int ShiftLane);

	UFUNCTION(BlueprintCallable, Category = Shield)
	void ActivateShield();

	UFUNCTION()
	void DeactivateShield();

	UMaterialInstanceDynamic* BodyMaterial;

	FTimerHandle ShieldTimerHandle;

	FVector SetAim(FVector worldLocation, FVector worldDirection);

	void StartFire();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void Fire(FVector aimLoc);

	void TurnCorner();

	UFUNCTION(Category=Control)
	void SlideStarted();

	UFUNCTION()
	void SlideEnded(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


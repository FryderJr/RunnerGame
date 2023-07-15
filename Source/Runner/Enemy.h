// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy.generated.h"

class USkeletalMeshComponent;
class UCapsuleComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum EEnemyTypes
{
	Cover	UMETA(DisplayName = "Covering"),
	Crouch	UMETA(DisplayName = "Crouching"),
	Stand	UMETA(DisplayName = "Standing")
};

UCLASS()
class RUNNER_API AEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemy();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void Start();

	void Crouch();

	void Uncrouch();

	AActor* Target;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float fireRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	bool isCrouching;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TEnumAsByte<EEnemyTypes> Type;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> Projectile;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RotateTowardsTarget();

	void Fire();

	UFUNCTION()
	void OnEnemyHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnEnemyDetected(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEnterFireRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UBoxComponent* FireRange;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UBoxComponent* EnemyDetectionRange;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* GunMeshComponent;

	bool isDead;

	float lastFired;

	float defaultHeight;

	FVector DefaultLocation;

	FRotator DefaultRotation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy.generated.h"

class USkeletalMeshComponent;
class UCapsuleComponent;
class UBoxComponent;

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

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	FVector DefaultLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	FRotator DefaultRotation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float fireRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	bool isCrouching;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> Projectile;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RotateTowardsTarget();

	void Fire();

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnEnemyDetected(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* GunMeshComponent;

	float lastFired;

	float defaultHeight;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

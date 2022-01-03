// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

// Sets default values
AEnemy::AEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	RootComponent = CapsuleComponent;
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	//MeshComponent->SetRelativeLocationAndRotation(DefaultLocation, DefaultRotation.Quaternion(), false, nullptr, ETeleportType::TeleportPhysics);
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("EnemyDetector"));
	BoxComponent->SetupAttachment(RootComponent);
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Gun"));
	GunMeshComponent->SetupAttachment(MeshComponent);
}

void AEnemy::Start()
{
	Target = nullptr;
	MeshComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true));
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetRelativeLocationAndRotation(DefaultLocation, DefaultRotation, false, nullptr, ETeleportType::ResetPhysics);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	CapsuleComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Crouch();
	
}

void AEnemy::Crouch()
{
	isCrouching = true;
	CapsuleComponent->SetCapsuleHalfHeight(defaultHeight / 2.0f);
	CapsuleComponent->MoveComponent(FVector(0.0f, 0.0f, -defaultHeight / 2.0f), CapsuleComponent->GetComponentRotation(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::ResetPhysics);
	MeshComponent->SetRelativeLocationAndRotation(FVector(DefaultLocation.X, DefaultLocation.Y, DefaultLocation.Z + (defaultHeight / 2.0f)), DefaultRotation, false, nullptr, ETeleportType::ResetPhysics);
}

void AEnemy::Uncrouch()
{
	if (!isCrouching)
	{
		return;
	}
	isCrouching = false;
	CapsuleComponent->SetCapsuleHalfHeight(defaultHeight);
	//CapsuleComponent->SetRelativeLocationAndRotation(FVector(DefaultLocation.X, DefaultLocation.Y, DefaultLocation.Z), DefaultRotation.Quaternion(), false, nullptr, ETeleportType::TeleportPhysics);
	CapsuleComponent->MoveComponent(FVector(0.0f, 0.0f, defaultHeight / 2.0f), CapsuleComponent->GetComponentRotation(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	MeshComponent->SetRelativeLocationAndRotation(DefaultLocation, DefaultRotation, false, nullptr, ETeleportType::ResetPhysics);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	defaultHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	DefaultLocation = MeshComponent->GetRelativeLocation();
	DefaultRotation = MeshComponent->GetRelativeRotation();
	print(FString::Printf(TEXT("Default location is (%f, %f, %f)"), DefaultLocation.X, DefaultLocation.Y, DefaultLocation.Z));
	print(FString::Printf(TEXT("Default rotation is (%f, %f, %f)"), DefaultRotation.Roll, DefaultRotation.Pitch, DefaultRotation.Yaw));
	
	GunMeshComponent->AttachTo(MeshComponent, WeaponSocketName, EAttachLocation::SnapToTarget, false);
	CapsuleComponent->OnComponentHit.AddDynamic(this, &AEnemy::OnCapsuleHit);
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnEnemyDetected);
	Start();
}

void AEnemy::RotateTowardsTarget()
{
	if (Target == nullptr)
	{
		return;
	}
	SetActorRotation(FRotator(0.0f, UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation()).Yaw, 0.0f));
}

void AEnemy::Fire()
{
	if (Target == nullptr)
	{
		return;
	}
	float currentTime = GetGameTimeSinceCreation();
	if (currentTime - lastFired < fireRate)
	{
		return;
	}
	FVector muzzleLoc = GunMeshComponent->GetSocketLocation(MuzzleSocketName);
	FVector targetLoc = Target->GetActorLocation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(Projectile, muzzleLoc, UKismetMathLibrary::FindLookAtRotation(muzzleLoc, targetLoc), SpawnParams);
	lastFired = currentTime;
}

void AEnemy::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->IsA(Projectile))
	{
		print(FString::Printf(TEXT("Hit occured!")));
		MeshComponent->SetSimulatePhysics(true);
		Target = nullptr;
		CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
}

void AEnemy::OnEnemyDetected(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* characterTemp = Cast<ACharacter>(OtherActor);
	if (characterTemp == nullptr)
	{
		return;
	}
	Target = OtherActor;
	Uncrouch();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateTowardsTarget();
	Fire();
}


// Copyright 2020 Stephen Maloney

#include "Equipment/ProjectileActor.h"

#include "Character/CeremonyCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "DrawDebugHelpers.h"
#include "Core/CeremonyFunctionLibrary.h"

AProjectileActor::AProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionProfileName("OverlapOnlyPawn");
	SphereComponent->GetBodyInstance()->bNotifyRigidBodyCollision = true;
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SetRootComponent(SphereComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->SetCollisionProfileName("NoCollision");
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

	// A long bow can shoot at 225 feet per second, which is approximately 6858 cm/s.
	ProjectileMovementComponent->InitialSpeed = 6858.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	
	SetReplicates(true);
}

void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACeremonyCharacter>(GetOwner());

	if(OwnerCharacter->IsLocallyControlled())
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileActor::OnSphereComponentBeginOverlap);
		SphereComponent->OnComponentHit.AddDynamic(this, &AProjectileActor::OnSphereComponentHit);

		if(bShowDebug)
		{
			SetActorTickEnabled(true);
		}
	}
}

void AProjectileActor::OnSphereComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(bShowDebug)
	{
		const FString InfoString = FString::Printf(TEXT("AProjectileActor::OnSphereComponentBeginOverlap : Other Actor %s Other Component %s"), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));
		UCeremonyFunctionLibrary::LogRoleAndMode(OwnerCharacter, InfoString);
	}

	ACeremonyCharacter* CharacterHit = Cast<ACeremonyCharacter>(OtherActor);
	
	if(!IsValid(CharacterHit) || CharacterHit == OwnerCharacter)
	{
		return;
	}
		
	//const FVector_NetQuantize DamageEnduranceDamageStunTime = FVector_NetQuantize::ZeroVector;
	//const uint8 DamageType = 0;
	//OwnerCharacter->Server_VerifyOverlapForDamage(CharacterHit, SphereComponent->GetComponentLocation(), DamageEnduranceDamageStunTime, DamageType);
	
	ProjectileMovementComponent->StopMovementImmediately();

	if(OtherActor->IsRootComponentMovable())
	{
		AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);
	}
	
	SetLifeSpan(5.0f);	
}

void AProjectileActor::OnSphereComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(bShowDebug)
	{
		const FString InfoString = FString::Printf(TEXT("AProjectileActor::OnSphereComponentHit : Other Actor %s Other Component %s"), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));
		UCeremonyFunctionLibrary::LogRoleAndMode(OwnerCharacter, InfoString);
	}

	ProjectileMovementComponent->StopMovementImmediately();

	if(OtherActor->IsRootComponentMovable())
	{
		AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);
	}
	
	SetLifeSpan(5.0f);	
}

void AProjectileActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		DrawDebugSphere(World, SphereComponent->GetComponentLocation(), 1.0f, 8, FColor::Red, false, 1.0f);		
	}
}

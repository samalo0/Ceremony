// Copyright 2020 Stephen Maloney

#include "Equipment/RangedWeaponActor.h"

#include "Components/ArrowComponent.h"
#include "Character/CeremonyCharacter.h"
#include "Equipment/ProjectileActor.h"
#include "Components/SkeletalMeshComponent.h"

ARangedWeaponActor::ARangedWeaponActor()
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->ArrowSize = 0.25f;
	SetRootComponent(ArrowComponent);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(GetRootComponent());
	SkeletalMeshComponent->bDisableClothSimulation = true;
	SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	SkeletalMeshComponent->bEnablePhysicsOnDedicatedServer = false;
	SkeletalMeshComponent->bDisableMorphTarget = true;
	SkeletalMeshComponent->bPerBoneMotionBlur = false;
	
	EquipmentState = EEquipmentStates::EquippedTwoHand;
	bMeleeLocomotion = false;
}

void ARangedWeaponActor::CancelActions()
{
	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARangedWeaponActor::CancelActions: Cancelling actions for character %s on %s."), *GetNameSafe(OwnerCharacter), *GetNameSafe(this));
	}

	bTwoHandPress1OnResume = false;
	bTwoHandPress1Held = false;
	
	OwnerCharacter->ClearOnMontageEndedDelegate();
	OwnerCharacter->StopMontageGlobally();
		
	OwnerCharacter->SetAllowEnduranceRecovery(true);
	OwnerCharacter->SetIsAttacking(false);
}

#pragma region Attack

void ARangedWeaponActor::CheckForAttackTransition()
{
	if(bTwoHandPress1IsPreparing)
	{
		bTwoHandPress1IsPreparing = false;
	}

	if(!bTwoHandPress1Held)
	{
		FireProjectile();
	}
}

void ARangedWeaponActor::OnAttackMontageEnded(UAnimMontage* Montage, const bool bInterrupted)
{
	OwnerCharacter->ClearOnMontageEndedDelegate();

	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARangedWeaponActor::OnAttackMontageEnded: Character %s objects %s Montage %s Interrupted %d"), *GetNameSafe(OwnerCharacter), *GetNameSafe(this), *GetNameSafe(Montage), bInterrupted);
	}

	OwnerCharacter->StopMontageGlobally();

	OwnerCharacter->SetAllowEnduranceRecovery(true);
	OwnerCharacter->SetIsAttacking(false);
	OwnerCharacter->CheckForResumingAction();
}

#pragma endregion

#pragma region TwoHandPress1

void ARangedWeaponActor::FireProjectile()
{
	const FVector_NetQuantize Location = GetActorLocation();
	const FRotator Rotation = FRotator(0.0f, OwnerCharacter->GetActorRotation().Yaw, 0.0f);
	Server_SpawnProjectile(ProjectileClassToSpawn, Location, Rotation);

	OwnerCharacter->SetIsAiming(false);
	OwnerCharacter->SetAllowEnduranceRecovery(false);
	OwnerCharacter->DepleteEndurance(Press1AttackParams.EnduranceConsumption);
	
	OwnerCharacter->PlayMontageGlobally(Press1AttackParams.AttackMontage);
	OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press1AttackParams.AttackMontage);

	// Reset animation on the bow
	SkeletalMeshComponent->SetPosition(0, false);
}

void ARangedWeaponActor::TwoHandPress1()
{
	if(OwnerCharacter->GetCanAttack())
	{
		OwnerCharacter->SetIsAttacking(true);
		OwnerCharacter->SetIsAiming(true);

		bTwoHandPress1IsPreparing = true;
		OwnerCharacter->PlayMontageGlobally(Press1AttackParams.PrepareMontage);

		// Play draw animation on the bow
		SkeletalMeshComponent->Play(false);
	}
	else
	{
		bTwoHandPress1OnResume = true;
	}

	bTwoHandPress1Held = true;
}

void ARangedWeaponActor::TwoHandRelease1()
{
	bTwoHandPress1Held = false;

	if(!bTwoHandPress1IsPreparing)
	{
		FireProjectile();
	}
}

void ARangedWeaponActor::TwoHandResume1(bool bHeldDown)
{
	if(bTwoHandPress1OnResume)
	{
		bTwoHandPress1OnResume = false;
		TwoHandPress1();
	}
}

#pragma endregion

#pragma region Server

void ARangedWeaponActor::Server_SpawnProjectile_Implementation(TSubclassOf<AProjectileActor> ClassToSpawn,
	const FVector_NetQuantize Location, const FRotator Rotation)
{
	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("ARangedWeaponActor::Server_SpawnProjectile_Implementation: Can't spawn projectile, world invalid."));
		return;
	}
		
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AProjectileActor* Projectile = World->SpawnActor<AProjectileActor>(ProjectileClassToSpawn, Location, Rotation, SpawnParameters);
}

#pragma endregion
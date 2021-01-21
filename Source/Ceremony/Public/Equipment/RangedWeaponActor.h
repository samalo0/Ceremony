// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Equipment/WeaponActor.h"
#include "RangedWeaponActor.generated.h"

class AProjectileActor;
class UAnimMontage;
class UArrowComponent;

/**
 *  Base class for ranged weapons in Ceremony.
 */
UCLASS()
class CEREMONY_API ARangedWeaponActor : public AWeaponActor
{

	GENERATED_BODY()

public:

	ARangedWeaponActor();

	void CancelActions() override;
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "RangedWeapon")
	TSubclassOf<AProjectileActor> ProjectileClassToSpawn;
	
	// Set to show debug messaging.
	UPROPERTY(EditDefaultsOnly, Category = "RangedWeapon")
	bool bShowDebugMessages = false;

#pragma region Attack

public:
	
	void CheckForAttackTransition() override;

protected:
	
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
#pragma endregion 
	
#pragma region Components

protected:
	
	// Root component for locating where the weapon will attach to the character.
	UPROPERTY(VisibleAnywhere)
	UArrowComponent* ArrowComponent;

	// Mesh for the visible part of the weapon.
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* SkeletalMeshComponent;
	
#pragma endregion

#pragma region TwoHandPress1

public:
	
	// Draws an projectile; hold to aim.
	void TwoHandPress1() override;

	// Release the projectile.
	void TwoHandRelease1() override;

	// On resume, execute press 1 if it was pressed during another action.
	void TwoHandResume1(bool bHeldDown) override;
	
protected:

	void FireProjectile();

	bool bTwoHandPress1IsPreparing = false;
	
	// If press 1 occurs while executing another action, resume it when the action finishes.
	bool bTwoHandPress1OnResume = false;

	bool bTwoHandPress1Held = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "RangedWeapon | Press1")
	FRangedAttackParams Press1AttackParams;

#pragma endregion

#pragma region Server

	// Spawns the projectile on the server.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnProjectile(TSubclassOf<AProjectileActor> ClassToSpawn, FVector_NetQuantize Location, FRotator Rotation);
	void Server_SpawnProjectile_Implementation(TSubclassOf<AProjectileActor> ClassToSpawn, FVector_NetQuantize Location, FRotator Rotation);
	bool Server_SpawnProjectile_Validate(TSubclassOf<AProjectileActor> ClassToSpawn, FVector_NetQuantize Location, FRotator Rotation) { return true; }	
	
	
#pragma endregion
	
};

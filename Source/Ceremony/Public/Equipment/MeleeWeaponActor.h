// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "MeleeWeaponActor.generated.h"

/**
 * Base class for melee weapons in Ceremony.
 */
UCLASS()
class CEREMONY_API AMeleeWeaponActor : public AWeaponActor
{

	GENERATED_BODY()

public:

	AMeleeWeaponActor();

	// Interrupt and cancel all actions the weapon can take.
	void CancelActions() override;
	
	void ShowCollision(bool bShow) override;
	
protected:

	// Set to show debug messaging.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon")
	bool bShowDebugMessages = false;
	
#pragma region Attack

public:

	// Called from animation notify to allow transitioning to secondary attacks.
	void CheckForAttackTransition() override;
	
	// Enables the collision on the weapon to trigger hits.
	void SetAttackCanDamage(bool bCanDamage) override;

protected:

	// The function to call when an attack animation finishes playing.
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, const bool bInterrupted);

	// The capsule collision component for the weapon is enabled/disabled based on notifies from animations.
	UFUNCTION()
	void OnCapsuleComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                               const FHitResult& SweepResult);

	// Array to keep track of actors damaged per attack, to prevent multiple damages from one attack.
	UPROPERTY(Transient)
	TArray<AActor*> DamagedActors;

	// Vector which stores the damage, endurance damage, and stun time that will be inflicted on a capsule overlap.
	FVector_NetQuantize DamageEnduranceDamageStunTime = FVector::ZeroVector;

	// Damage type to inflict on capsule overlap.
	uint8 DamageType;
	
#pragma endregion
	
#pragma region Components

protected:
	
	// Root component for locating where the weapon will attach to the character.
	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* ArrowComponent;
	
	// Capsule collision for the part of the weapon that can damage other characters.
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* CapsuleComponent;
	
	// Mesh for the visible part of the weapon.
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

#pragma endregion
	
#pragma region Press1

public:

	// Standard attack.
	void Press1() override;

	// Standard attack has no release behavior. It's triggered by press.
	void Release1() override {}

	// On resume, execute press 1 if it was pressed during another action.
	void Resume1(bool bHeldDown) override;
	
protected:
	
	// The parameters for standard attack.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | Press1")
	TArray<FStandardAttackParams> Press1AttackParams;

	// The parameters for the standard attack while running.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | Press1")
	FStandardAttackParams Press1RunningAttackParams;
	
	// Keeps track of which attack is active from the press1 attack characteristics array.
	int32 Press1CurrentAttack = 0;

	// Keeps track if an attack is in progress.
	bool bPress1IsAttacking = false;

	// If press 1 occurs while executing another action, resume it when the action finishes.
	bool bPress1OnResume = false;
	
	// Keeps track if another attack should be triggered at the end of the current attack.
	bool bPress1QueueNextAttack = false;
	
#pragma endregion

#pragma region Press2

public:

	// Charge attack.
	void Press2() override;

	// Charge attack is released to perform the attack.
	void Release2() override;

	// Charge attack can be triggered on resume if it's held down.
	void Resume2(bool bHeldDown) override;
	
protected:

	// Separate function from Press2 in order to allow triggering it later; if a press1 attack is active, and you press2, transition to it as soon as press1 ends.
	void TriggerPress2Attack();

	// If charging or attacking with press2.
	bool bPress2IsAttacking = false;

	// If currently charging the press2 attack.
	bool bPress2IsCharging = false;

	// Keeps track if the press 2 button is held down, for transitioning attacks.
	bool bPress2IsHeldDown = false;

	// Keeps track if press 2 has been pressed while another attack is active to transition to it immediately.
	bool bPress2QueueNextAttack = false;
	
	// Parameters for the charge attack that occurs while pressing 2.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | Press2")
	FChargedAttackCharacteristic Press2AttackParams;

	// Records the time that a charge attack starts.
	float Press2ChargeStartTimestamp = 0.0f;

	// Handle used to force a charged attack if you hold it down longer than the maximum charge time.
	FTimerHandle Press2FullyChargedTimerHandle;
	
	// Parameters for the jumping attack that occurs while running and pressing 2.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | Press2")
	FStandardAttackParams Press2JumpingAttackParams;
	
#pragma endregion

#pragma region SpecialAttack

protected:

	ACeremonyCharacter* CheckForSpecialAttack(ESpecialAttackType& OutAttackType) const;
	
	// When attempting a back stab, the dot product of the two forward vectors is taken and compared to make sure both characters are facing in the right location. Face to back is 1.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin = 0.0f, ClampMax = 0.99f), Category = "MeleeWeapon | SpecialAttack")
	float BackStabDotProductMinimum = 0.8f;

	// The amount of endurance consumed when executing a back stab, whether successful or not.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin = 0.0f), Category = "MeleeWeapon | SpecialAttack")
	float BackStabEnduranceConsumption = 20.0f;

	// Montage to play when executing a back stab.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | SpecialAttack")
	UAnimMontage* BackStabMontage;
	
	// When attempting a riposte, the dot product of the two forward vectors is taken and compare to make sure the characters are facing the right direction. Face to face is -1.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=-1.0f, ClampMax=0.0f), Category = "MeleeWeapon | SpecialAttack")
	float RiposteDotProductMaximum = -0.8f;

	// Endurance consumed when executing a riposte.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f), Category = "MeleeWeapon | SpecialAttack")
	float RiposteEnduranceConsumption = 20.0f;

	// Montage to play when executing a riposte.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | SpecialAttack")
	UAnimMontage* RiposteMontage;
	
	// The maximum distance at which a character can perform special attacks.
	UPROPERTY(EditDefaultsOnly, Category = "MeleeWeapon | SpecialAttack")
	float SpecialAttackReach = 100.0f;
	
#pragma endregion
	
};

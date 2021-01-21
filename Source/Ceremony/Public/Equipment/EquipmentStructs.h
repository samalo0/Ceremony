// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "EquipmentStructs.generated.h"

class UAnimMontage;

/**
 * Enumeration for all damage types that can be inflicted.
 */
UENUM(BlueprintType)
enum class EDamageTypes : uint8
{
	Bludgeon,
	Slash,
	Pierce,

	Kick,
};

/**
 * Structure for the state of the equipment.
 */
UENUM(BlueprintType)
enum class EEquipmentStates : uint8
{
	None,
	EquippedRightHand,
	EquippedLeftHand,
	EquippedTwoHand,
};

UENUM(BlueprintType)
enum class ESpecialAttackType : uint8
{
	None,
	BackStab,
	Riposte
};

/**
 * Structure to store the characteristics of possible damage.
 */
USTRUCT(BlueprintType)
struct FDamageParams
{
	GENERATED_BODY()

	// Damage multiplier for a successful back stab from a standard attack, multiplies the standard damage.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f, ClampMax=10.0f))
	float BackStabMultiplier = 3.0f;

	// The minimum amount of damage to do on charged attacks, or the standard attack damage.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float DamageStandard = 10.0f;

	// The maximum amount of damage to do on charged attacks. NOT USED ON STANDARD, JUMPING, AND RUNNING ATTACKS.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float DamageFullyCharged = 30.0f;

	// The damage type of the attack.
	UPROPERTY(EditDefaultsOnly)
	EDamageTypes DamageType = EDamageTypes::Slash;
	
	// The minimum amount of endurance damage to do to a shield on charged attacks, or the amount for standard attacks.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float EnduranceDamageStandard = 40.0f;

	// The maximum amount of endurance damage to do to a shield on charged attacks. NOT USED ON STANDARD, JUMPING, AND RUNNING ATTACKS.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float EnduranceDamageFullyCharged = 80.0f;

	// Damage multiplier for a successful riposte, multiplies damage minimum.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f, ClampMax=10.0f))
	float RiposteMultiplier = 4.0f;
	
	// Maximum time to stun the player when hit with the attack.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float StunTime = 0.5f;
};

/**
 * Structure for charged attack characteristics. These attacks allow you to hold down the button for a period
 * of time, and when released execute an attack scaled on the amount of time it was held.
 */
USTRUCT(BlueprintType)
struct FChargedAttackCharacteristic
{
	GENERATED_BODY()

	// The animation montage to play for attacking.
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* AttackMontage;

	// The animation montage to play for charging; which should loop.
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ChargeMontage;
	
	// Amount of time to charge for maximum damage.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float ChargeSeconds = 3.0f;
	
	// Damage characteristics.
	UPROPERTY(EditDefaultsOnly)
	FDamageParams DamageParams;

	// Amount of endurance consumed at maximum charge time.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float EnduranceConsumptionMaximum = 50.0f;
	
	// Amount of endurance consumed at minimum charge time.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float EnduranceConsumptionMinimum = 20.0f;
};

/**
 * Structure for standard attack characteristics. These are attacks that are triggered by a single button press.
 */
USTRUCT(BlueprintType)
struct FStandardAttackParams
{
	GENERATED_BODY()
	
	// Damage characteristics of the attack.
	UPROPERTY(EditDefaultsOnly)
	FDamageParams DamageParams;

	// Amount of endurance consumed by executing the attack.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float EnduranceConsumption = 20.0f;

	// The animation montage to play for the attack.
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* Montage;

	// The section name in the montage to jump to if attacks are chained.
	UPROPERTY(EditDefaultsOnly)
	FName MontageActiveJumpSectionName = NAME_None;
};

/**
 * Structure for standard attack characteristics. These are attacks that are triggered by a single button press.
 */
USTRUCT(BlueprintType)
struct FRangedAttackParams
{
	GENERATED_BODY()
	
	// Damage characteristics of the attack.
	UPROPERTY(EditDefaultsOnly)
	FDamageParams DamageParams;

	// Amount of endurance consumed by executing the attack.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f))
	float EnduranceConsumption = 20.0f;

	// The animation montage to play when executing the attack.
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* AttackMontage;
	
	// The animation montage to play when preparing for the attack.
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* PrepareMontage;
};

/**
 * Structure to contain the blocking characteristics of a shield.
 */
USTRUCT()
struct FShieldBlockParams
{
	GENERATED_BODY()

	// The amount of physical damage taken while blocking = Damage * (1 - PhysicalDefense)
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f, ClampMax = 1.0f))
	float PhysicalDefense = 0.5f;

	// The amount of endurance damage done while blocking = EnduranceDamage * (1 - Stability)
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f, ClampMax = 1.0f))
	float Stability = 0.3f;
};
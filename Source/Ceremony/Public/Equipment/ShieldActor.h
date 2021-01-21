// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "EquipmentActor.h"
#include "ShieldActor.generated.h"

/**
 * Base class for shields.
 */
UCLASS()
class CEREMONY_API AShieldActor : public AEquipmentActor
{

	GENERATED_BODY()

public:

	AShieldActor();

	void CancelActions() override;

	void GetDamageAfterAbsorption(float DamageIn, EDamageTypes DamageType, float EnduranceDamageIn, float& DamageOut, float& EnduranceDamageOut) const;
	
#pragma region Components

protected:
	
	// Arrow root component, used to line up attachment to the hand..
	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* ArrowComponent;

	// Static mesh for the shield.
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;
	
#pragma endregion 

#pragma region Press1

public:

	// Block.
	void Press1() override;

	// Release block.
	void Release1() override;

	// Resume blocking if block is held down while completing another action.
	void Resume1(bool bHeldDown) override;

	// If hit while blocking, and not staggered, show impact.
	void ShowBlockImpact();
	
protected:

	UFUNCTION()
	void OnImpactMontageEnded(UAnimMontage* Montage, bool bInterrupted) const;
	
	// Montage to play to initiate block, idle, and release block
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press1")
	UAnimMontage* BlockingMontage;

	// The section to jump to in the blocking montage when block is released.
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press1")
	FName BlockMontageExitSectionName = NAME_None;

	// The section to jump to in the blocking montage when block is idle.
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press1")
	FName BlockMontageIdleSectionName = NAME_None;
	
	// Montage to play to show a shield impact while blocking.
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press1")
	UAnimMontage* ImpactMontage;
	
	// Shield blocking properties.
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press1")
	FShieldBlockParams ShieldBlockProps;
	
#pragma endregion

#pragma region Press2

public:

	// Parry.
	void Press2() override;

	// Parry has no release behavior.
	void Release2() override {}

	// Resume parry, if it's attempted while busy with another action.
	void Resume2(bool bHeldDown) override;
	
protected:

	// Function to call when the parry animation has finished playing.
	UFUNCTION()
	void OnParryMontageEnded(UAnimMontage* Montage, bool bInterrupted) const;

	// Amount of endurance consumed to parry.
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press2")
	float ParryEnduranceConsumption = 20.0f;
	
	// Montage to play to initiate parry.
	UPROPERTY(EditDefaultsOnly, Category = "ShieldActor | Press2")
	UAnimMontage* ParryMontage;

	// Remember if a parry is tried while executing other actions.
	bool bParryOnResume = false;
	
#pragma endregion
	
};

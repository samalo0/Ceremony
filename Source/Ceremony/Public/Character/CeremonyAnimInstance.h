// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CeremonyAnimInstance.generated.h"

/**
 * Base animation instance for the Ceremony character animation blueprint.
 */
UCLASS()
class CEREMONY_API UCeremonyAnimInstance : public UAnimInstance
{

	GENERATED_BODY()

public:

	void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsFalling;

	UPROPERTY(Transient, BlueprintReadOnly)
	float ForwardVelocity;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsLockedOn;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsNotFallingAndLockedOn;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsNotFallingAndNotLockedOn;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FVector LeftFootOffsetLocation;

	UPROPERTY(Transient, BlueprintReadOnly)
	FRotator LeftFootOffsetRotation;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bMeleeLocomotion;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FVector PelvisOffset;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector RightFootOffsetLocation;

	UPROPERTY(Transient, BlueprintReadOnly)
	FRotator RightFootOffsetRotation;

	UPROPERTY(Transient, BlueprintReadOnly)
	float RightVelocity;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	float Velocity;
		
};

// Copyright 2020 Stephen Maloney

#include "Character/CeremonyAnimInstance.h"

#include "Character/CeremonyCharacter.h"
#include "Character/CeremonyMovementComponent.h"
#include "Character/InverseKinematicsComponent.h"

void UCeremonyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	const auto Owner = Cast<ACeremonyCharacter>(TryGetPawnOwner());
	if(IsValid(Owner))
	{
		// Update character velocity to drive blend spaces.
		Velocity = Owner->GetVelocity().Size();

		// Get the IK offsets.
		Owner->InverseKinematicsComponent->GetOffsets(PelvisOffset, LeftFootOffsetLocation, RightFootOffsetLocation, LeftFootOffsetRotation,
		                                              RightFootOffsetRotation);

		bMeleeLocomotion = Owner->IsMeleeLocomotion();

		bIsFalling = Owner->GetCharacterMovement()->IsFalling();

		bIsLockedOn = Owner->GetIsLockedOn();

		bIsNotFallingAndLockedOn = !bIsFalling && bIsLockedOn;
		bIsNotFallingAndNotLockedOn = !bIsFalling && !bIsLockedOn;
		
		ForwardVelocity = FVector::DotProduct(Owner->GetVelocity(), Owner->GetActorForwardVector());
		RightVelocity = FVector::DotProduct(Owner->GetVelocity(), Owner->GetActorRightVector());
	}
}

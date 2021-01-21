// Copyright 2020 Stephen Maloney

#include "Character/CeremonyMovementComponent.h"
#include "Character/CeremonyCharacter.h"

void UCeremonyMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACeremonyCharacter>(GetOwner());
}

float UCeremonyMovementComponent::GetMaxSpeed() const
{
	auto MaxSpeed = Super::GetMaxSpeed();

	if(IsValid(OwnerCharacter))
	{
		if(OwnerCharacter->GetIsRunning())
		{
			MaxSpeed = OwnerCharacter->GetRunSpeed();
		}
		else
		{
			MaxSpeed = OwnerCharacter->GetWalkSpeed();
		}
	}

	return MaxSpeed;
}

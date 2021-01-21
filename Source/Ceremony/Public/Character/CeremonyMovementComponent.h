// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CeremonyMovementComponent.generated.h"

/**
 * Overloaded movement component to allow running on server and client.
 */
UCLASS()
class CEREMONY_API UCeremonyMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	void BeginPlay() override;
	
	float GetMaxSpeed() const override;

	// Character reference.
	UPROPERTY(Transient)
	class ACeremonyCharacter* OwnerCharacter;
	
};

// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/DataTable.h"

#include "FootstepComponent.generated.h"

class ACeremonyCharacter;

/**
 * Structure for a data table containing footstep sounds.
 */
USTRUCT(BlueprintType)
struct FFootStepSound : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* FootstepSound;
};

/**
 * Footstep sound effect component.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEREMONY_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UFootstepComponent();

	// Call to get the sound to play for the material which is below the character.
	USoundBase* GetFootstepSound(FName FootBoneName) const;
	
protected:

	void BeginPlay() override;

	// Query params to use over and over when checking the material.
	FCollisionQueryParams CollisionQueryParams;

	// Data table to use for looking up footstep sounds.
	UPROPERTY(EditDefaultsOnly)
	UDataTable* FootstepDataTable;
	
	// The distance to trace down from the foot to find a surface for footstep sounds.
	UPROPERTY(EditDefaultsOnly)
	float FootTraceLength = 30.0f;
	
	// Reference to the owning character.
	UPROPERTY(Transient)
	ACeremonyCharacter* OwnerCharacter;

	// Reference to the owning character skeletal mesh.
	UPROPERTY(Transient)
	USkeletalMeshComponent* OwnerMesh;

	// Show debug messages.
	UPROPERTY(EditDefaultsOnly)
	bool bShowDebug = false;
};

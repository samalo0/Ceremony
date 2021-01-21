// Copyright Stephen Maloney 2020

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CeremonyAnimNotify.generated.h"

class USoundBase;

/**
 * Enumeration for all types of singular notifies to fire for the character.
 */
UENUM(BlueprintType)
enum class ECeremonyAnimNotifyType : uint8
{
	CanTransitionToNextAttack = 0,
	PlaySound = 1,
	PlayFootstepSound = 2,
};

/**
 * Animation notify class for characters. Use this to trigger events that have no duration from animations.
 */
UCLASS()
class CEREMONY_API UCeremonyAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:

	UCeremonyAnimNotify();
	
#if WITH_EDITOR
	// Add checks for the editor so that you can only edit pertinent properties.
	bool CanEditChange(const FProperty* InProperty) const override;
#endif
	
protected:

	// The type of notify to trigger, set in the details panel.
	UPROPERTY(EditAnywhere)
	ECeremonyAnimNotifyType NotifyType;

	// If notifying of an attack transition, which hand is the weapon in?
	UPROPERTY(EditAnywhere)
	bool bCanTransitionRightHand = true;

	// If notifying a footstep sound, which foot?
	UPROPERTY(EditAnywhere)
	bool bRightFoot = true;
	
	// The sound to play if triggering sound playback.
	UPROPERTY(EditAnywhere)
	USoundBase* Sound;

	// Whether to play the sound for all players (true), or just on the locally controlled client (false).
	UPROPERTY(EditAnywhere)
	bool bPlaySoundMulticast = false;
	
private:

	// Returns the name displayed in the animation when this notify is placed.
	FString GetNotifyName_Implementation() const override;

	// Called when the notify is encountered during animation playback.
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};

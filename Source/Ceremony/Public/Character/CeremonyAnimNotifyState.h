// Copyright Stephen Maloney 2020

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CeremonyAnimNotifyState.generated.h"

/**
 * Enumeration of the possible states to select for this notify.
 */
UENUM(BlueprintType)
enum class ECeremonyAnimNotifyStateType : uint8
{
	Attack,
	Invincibility,
	Kick,
	Movement,
	Parry,
};

/**
 * When the notify state is movement, this is the type of movement allowed.
 */
UENUM(BlueprintType)
enum class EMovementType : uint8
{
	ContinuousInputDirection,
	ForcedForwardOrBack,
	ForcedLastInputDirection,
};

/**
 * Animation notify state class for characters. Use to trigger event types that have a duration, rather than a singular point.
 */
UCLASS()
class CEREMONY_API UCeremonyAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

	// Returns the name to display in the animation once this state notify is placed.
	FString GetNotifyName_Implementation() const override;

	// Called at the begin of this notify.
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	// Called on the end of this notify.
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:

#if WITH_EDITOR
	// Add checks for the editor so that you can only edit pertinent properties.
	bool CanEditChange(const FProperty* InProperty) const override;
#endif
	
protected:

	// When notifying that attack is active, is the weapon in the right hand?
	UPROPERTY(EditAnywhere)
	bool bAttackIsRightHanded = true;
	
	// The type of forced movement when locked on.
	UPROPERTY(EditAnywhere)
	EMovementType LockedMovementType;

	// The rate at which to move during forced movement frames. The movement control type is specified by MovementType.
	UPROPERTY(EditAnywhere, meta=(ClampMin=-1.0f, ClapmMax=1.0f))
	float MovementRate = 0.0f;
	
	// The type of notify, to allow this class to be reused for many state notify types.
	UPROPERTY(EditAnywhere)
	ECeremonyAnimNotifyStateType NotifyStateType;

	// The type of forced movement when unlocked.
	UPROPERTY(EditAnywhere)
	EMovementType UnlockedMovementType;
	
};

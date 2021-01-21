// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InverseKinematicsComponent.generated.h"

/**
 * Class for inverse kinematics for feet and hips.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEREMONY_API UInverseKinematicsComponent : public UActorComponent
{

	GENERATED_BODY()

public:	

	UInverseKinematicsComponent();

	FORCEINLINE FName GetLeftFootBoneName() const { return LeftFootBoneName; }
		
	// Gets the offsets for the pelvis and feet.
	FORCEINLINE void GetOffsets(FVector& OutPelvisOffset, FVector& OutLeftFootOffsetLocation,
	FVector& OutRightFootOffsetLocation, FRotator& OutLeftFootOffsetRotation, FRotator& OutRightFootOffsetRotation) const;

	FORCEINLINE FName GetRightFootBoneName() const { return RightFootBoneName; }
	
	FORCEINLINE bool GetShowDebugTraces() const { return bShowDebugTraces; }

	void SetShowDebugTraces(const bool bShowTraces) { bShowDebugTraces = bShowTraces; }
	
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:

	// Resets the offsets by interpolating to 0 offset.
	void ResetOffsets(float DeltaTime);
	
	// Sets the foot offset for the named foot and returns the current and target offsets.
	void SetFootOffset(float DeltaTime, FName FootBoneName, FVector& CurrentOffsetLocation, FVector& TargetOffsetLocation, FRotator& CurrentOffsetRotation) const;

	// Sets the pelvis offset.
	void SetPelvisOffset(float DeltaTime, FVector InLeftFootTargetOffsetLocation, FVector InRightFootTargetOffsetLocation);

	// Floor location offset to prevent feet from being stuck through the floor.
	UPROPERTY(EditDefaultsOnly)
	float FootFloorLocationOffset = -3.0f;
	
	// The height of the foot z above the root of the skeleton.
	UPROPERTY(EditDefaultsOnly)
	float FootHeight = 13.465573f;

	// How fast to interpolate to new values when moving down.
	UPROPERTY(EditDefaultsOnly)
	float InterpolationDownSpeed = 30.0f;
	
	// How fast to interpolate to new values when moving up.
	UPROPERTY(EditDefaultsOnly)
	float InterpolationUpSpeed = 15.0f;

	// The name of the left foot bone, for getting it's location.
	UPROPERTY(EditDefaultsOnly)
	FName LeftFootBoneName = "foot_l";

	FVector LeftFootCurrentOffsetLocation = FVector::ZeroVector;
	FRotator LeftFootCurrentOffsetRotation = FRotator::ZeroRotator;
	FVector LeftFootTargetOffsetLocation = FVector::ZeroVector;
	
	FVector PelvisOffset = FVector::ZeroVector;
	FVector PelvisTarget = FVector::ZeroVector;
	
	// The name of the right foot bone, for getting it's location.
	UPROPERTY(EditDefaultsOnly)
	FName RightFootBoneName = "foot_r";

	FVector RightFootCurrentOffsetLocation = FVector::ZeroVector;
	FVector RightFootTargetOffsetLocation = FVector::ZeroVector;
	FRotator RightFootCurrentOffsetRotation = FRotator::ZeroRotator;
	
	// Show debug traces for locations, impact points, and target locations.
	UPROPERTY(EditDefaultsOnly)
	bool bShowDebugTraces = false;

	// How far above the foot floor to start the trace.
	UPROPERTY(EditDefaultsOnly)
	float TraceDistanceAbove = 30.0f;

	// How far below the foot floor to end the trace.
	UPROPERTY(EditDefaultsOnly)
	float TraceDistanceBelow = 30.0f;
};

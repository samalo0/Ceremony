// Copyright 2020 Stephen Maloney

#include "Character/InverseKinematicsComponent.h"

#include "Character/CeremonyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UInverseKinematicsComponent::UInverseKinematicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UInverseKinematicsComponent::GetOffsets(FVector& OutPelvisOffset, FVector& OutLeftFootOffsetLocation,
	FVector& OutRightFootOffsetLocation, FRotator& OutLeftFootOffsetRotation, FRotator& OutRightFootOffsetRotation) const
{
	OutPelvisOffset = PelvisOffset;
	OutLeftFootOffsetLocation = LeftFootCurrentOffsetLocation;
	OutRightFootOffsetLocation = RightFootCurrentOffsetLocation;
	OutLeftFootOffsetRotation = LeftFootCurrentOffsetRotation;
	OutRightFootOffsetRotation = RightFootCurrentOffsetRotation;
}

void UInverseKinematicsComponent::ResetOffsets(const float DeltaTime)
{
	SetPelvisOffset(DeltaTime, FVector::ZeroVector, FVector::ZeroVector);

	LeftFootCurrentOffsetLocation = FMath::VInterpTo(LeftFootCurrentOffsetLocation, FVector::ZeroVector, DeltaTime, InterpolationDownSpeed);
	RightFootCurrentOffsetLocation = FMath::VInterpTo(RightFootCurrentOffsetLocation, FVector::ZeroVector, DeltaTime, InterpolationDownSpeed);
}

void UInverseKinematicsComponent::SetFootOffset(float DeltaTime, FName FootBoneName, FVector& CurrentOffsetLocation, FVector& TargetOffsetLocation, FRotator& CurrentOffsetRotation) const
{
	const ACeremonyCharacter* Owner = Cast<ACeremonyCharacter>(GetOwner());
	const UWorld* World = GetWorld();
	if(!IsValid(Owner) || !IsValid(World))
	{
		return;
	}

	// Get the location of the foot bone in world space.
	const FVector FootBoneLocation = Owner->GetMesh()->GetSocketLocation(FootBoneName);
	
	// Get the location of the skeletal mesh in world space.
	const FVector RootLocation = Owner->GetMesh()->GetComponentLocation();
	
	// Build a vector that is the foot X and Y and the skeletal mesh Z location in world space.
	const FVector FootFloorLocation = FVector(FootBoneLocation.X, FootBoneLocation.Y, RootLocation.Z + FootFloorLocationOffset);

	// Build starting and ending vectors which are offsets above and below the foot floor location.
	const FVector AboveFootFloorLocation = FVector(FootFloorLocation.X, FootFloorLocation.Y, FootFloorLocation.Z + TraceDistanceAbove);
	const FVector BelowFootFloorLocation = FVector(FootFloorLocation.X, FootFloorLocation.Y, FootFloorLocation.Z - TraceDistanceBelow);

	FHitResult OutHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	FRotator TargetOffsetRotation = FRotator::ZeroRotator;
	
	if(World->LineTraceSingleByChannel(OutHit, AboveFootFloorLocation, BelowFootFloorLocation, ECC_Visibility, Params))
	{
		// Find the target offset in world space - the difference between the impact point and where it would be normally with no offset.
		TargetOffsetLocation = ((OutHit.ImpactNormal * FootHeight) + OutHit.ImpactPoint) - (FootFloorLocation + FVector(0.0f, 0.0f, FootHeight));
		
		// Find the target foot rotation offset in degrees.
		TargetOffsetRotation = FRotator(-1.0f * FMath::RadiansToDegrees(FMath::Atan2(OutHit.ImpactNormal.X, OutHit.ImpactNormal.Z)),
			0.0f, FMath::RadiansToDegrees(FMath::Atan2(OutHit.ImpactNormal.Y, OutHit.ImpactNormal.Z)));
		
		if(bShowDebugTraces)
		{
			DrawDebugLine(World, AboveFootFloorLocation, BelowFootFloorLocation, FColor::Red, false, -1, 1, 0);
			DrawDebugBox(World, FootFloorLocation, FVector(1.0f), FQuat::Identity, FColor::Red, false, -1, 1, 0);
			
			DrawDebugBox(World, OutHit.ImpactPoint, FVector(1.0f), FQuat::Identity, FColor::Green, false, -1, 1, 0);
			DrawDebugLine(World, OutHit.ImpactPoint, FootFloorLocation, FColor::Green, false, -1, 2, 1);

			DrawDebugLine(World, OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * FootHeight, FColor::Yellow, false, -1, 1, 0);
		}
	}
	else
	{
		TargetOffsetLocation = FVector::ZeroVector;
	}
	
	// Interpolate foot location.
	const float InterpolationSpeed = CurrentOffsetLocation.Z > TargetOffsetLocation.Z? InterpolationDownSpeed : InterpolationUpSpeed;
	CurrentOffsetLocation = FMath::VInterpTo(CurrentOffsetLocation, TargetOffsetLocation, DeltaTime, InterpolationSpeed);

	// Interpolate foot rotation.
	CurrentOffsetRotation = FMath::RInterpTo(CurrentOffsetRotation, TargetOffsetRotation, DeltaTime, InterpolationDownSpeed);
}

void UInverseKinematicsComponent::SetPelvisOffset(const float DeltaTime, const FVector InLeftFootTargetOffsetLocation,
	const FVector InRightFootTargetOffsetLocation)
{
	// Set the pelvis target to the lowest foot offset.
	PelvisTarget = InLeftFootTargetOffsetLocation.Z < InRightFootTargetOffsetLocation.Z? InLeftFootTargetOffsetLocation : InRightFootTargetOffsetLocation;

	const float InterpolationSpeed = PelvisOffset.Z > PelvisTarget.Z? InterpolationDownSpeed : InterpolationUpSpeed;
	PelvisOffset = FMath::VInterpTo(PelvisOffset, PelvisTarget, DeltaTime, InterpolationSpeed);

	if(bShowDebugTraces)
	{
		const ACeremonyCharacter* Owner = Cast<ACeremonyCharacter>(GetOwner());
		const UWorld* World = GetWorld();
		if(IsValid(Owner))
		{
			const FVector PelvisLocation = Owner->GetMesh()->GetSocketLocation("pelvis");

			DrawDebugBox(World, PelvisLocation, FVector(1.0f), FQuat::Identity, FColor::Green, false, -1, 1, 0);
			DrawDebugBox(World, PelvisLocation - PelvisOffset, FVector(1.0f), FQuat::Identity, FColor::Red, false, -1, 1, 0);
			DrawDebugLine(World, PelvisLocation, PelvisLocation - PelvisOffset, FColor::Green, false, -1, 2, 1);
		}
	}
}

void UInverseKinematicsComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACeremonyCharacter* Owner = Cast<ACeremonyCharacter>(GetOwner());
	if(!IsValid(Owner))
	{
		return;
	}

	if(Owner->GetCharacterMovement()->IsFalling())
	{
		ResetOffsets(DeltaTime);
	}
	else
	{
		SetFootOffset(DeltaTime, LeftFootBoneName, LeftFootCurrentOffsetLocation, LeftFootTargetOffsetLocation, LeftFootCurrentOffsetRotation);
		SetFootOffset(DeltaTime, RightFootBoneName, RightFootCurrentOffsetLocation, RightFootTargetOffsetLocation, LeftFootCurrentOffsetRotation);
		SetPelvisOffset(DeltaTime, LeftFootTargetOffsetLocation, RightFootTargetOffsetLocation);
	}
}


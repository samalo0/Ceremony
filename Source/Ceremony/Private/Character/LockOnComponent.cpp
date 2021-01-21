// Copyright Stephen Maloney 2020

#include "Character/LockOnComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/CeremonyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACeremonyCharacter>(GetOwner());
	check(IsValid(OwnerCharacter));
}

void ULockOnComponent::ClearLockedOn()
{
	if(IsValid(LockedOnCharacter))
	{
		LockedOnCharacter->SetOpponentHasLockedOn(false);	
	}
	LockedOnCharacter = nullptr;

	SetComponentTickEnabled(false);
	
	if(IsValid(OwnerCharacter))
	{
		OwnerCharacter->SetIsLockedOn(false);	
	}
}

ACeremonyCharacter* ULockOnComponent::FindValidCharacterWithHighestDotProduct()
{
	// Find the one closest to the center of the camera, which is the highest dot product.
	if(ValidLockOnCharacters.Num() > 0)
	{
		uint32 ClosestIndex = 0;
		float ClosestValue = ValidLockOnCharacters[0].DotProduct;

		for(auto Index = 1; Index < ValidLockOnCharacters.Num(); Index++)
		{
			if(ValidLockOnCharacters[Index].DotProduct > ClosestValue)
			{
				ClosestValue = ValidLockOnCharacters[Index].DotProduct;
				ClosestIndex = Index;
			}
		}

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::FindValidPawnWithHighestDotProduct: Selecting pawn %s."), *GetNameSafe(ValidLockOnCharacters[ClosestIndex].Character));
		}
		
		return ValidLockOnCharacters[ClosestIndex].Character;
	}

	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::FindValidPawnWithHighestDotProduct: Unable to find a pawn."));
	}
	
	return nullptr;
}

void ULockOnComponent::GetValidLockOnCharacters()
{
	const UWorld* World = GetWorld();
	if(!IsValid(World) || !IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("ULockOnComponent: World or owning character INVALID."));
		return;
	}

	// Empty out the list of valid lock on targets.
	ValidLockOnCharacters.Empty();

	// Cache owner location.
	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	
	TArray<FOverlapResult> OutOverlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	// Draw debug sphere to show the range.
	if(OwnerCharacter->IsShowingDebugCollision())
	{
		DrawDebugSphere(World, OwnerLocation, LockOnSphereRadius, 16, FColor::Red, false, 3.0f, 0, 0);
	}
	
	// First do a sphere overlap to find all characters within the valid radius.
	if(!World->OverlapMultiByObjectType(OutOverlaps, OwnerLocation, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(LockOnSphereRadius), Params))
	{
		// No pawns found in radius
		return;
	}
	
	for(FOverlapResult& OutOverlap : OutOverlaps)
	{
		ACeremonyCharacter* TargetCharacter = Cast<ACeremonyCharacter>(OutOverlap.GetActor());
		if(!IsValid(TargetCharacter) || TargetCharacter->GetHealth() == 0.0f)
		{
			continue;
		}

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::GetValidLockOnPawns: Checking actor %s, component %s for validity."), *GetNameSafe(OutOverlap.GetActor()), *GetNameSafe(OutOverlap.GetComponent()));
		}
		
		// Check that the cross product between the camera forward vector and the vector to the target is within range.
		const FRotator CameraRotation = FRotator(0.0f, OwnerCharacter->GetControlRotation().Yaw, 0.0f);
		const FVector CameraDirection = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::X);

		// Create a vector from the character to the target.
		FVector TargetVector = TargetCharacter->GetActorLocation() - OwnerLocation;
		TargetVector.Z = 0.0f;
		TargetVector = TargetVector.GetSafeNormal(0.1f);
		
		const FVector CrossProduct = FVector::CrossProduct(CameraDirection, TargetVector); 
		const float DotProduct = FVector::DotProduct(CameraDirection, TargetVector);

		// Check that the dot product is within range, to prevent locking onto characters that are behind the camera.
		if(DotProduct < DotProductRange)
		{
			if(OwnerCharacter->IsShowingDebugCollision())
			{
				DrawDebugLine(World, OwnerLocation, TargetCharacter->GetActorLocation(), FColor::Red, false, 3, 0, 0);
			}

			continue;
		}

		if(OwnerCharacter->IsShowingDebugCollision())
		{
			DrawDebugLine(World, OwnerLocation, TargetCharacter->GetActorLocation(), FColor::Green, false, 3, 0, 0);
		}

		ValidLockOnCharacters.Add(FValidLockOnCharacter(TargetCharacter, CrossProduct, DotProduct));
	}

	if(ValidLockOnCharacters.Num() > 1)
	{
		ValidLockOnCharacters.Sort();
	}

	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::GetValidLockOnCharacters: Found %d valid characters."), ValidLockOnCharacters.Num());
		for(FValidLockOnCharacter& LOC : ValidLockOnCharacters)
		{
			UE_LOG(LogTemp, Warning, TEXT("Pawn %s - Cross Product X %f, Y %f, Z %f, Dot Product %f, NetRole %d, NetMode %d."), *GetNameSafe(LOC.Character), LOC.CrossProduct.X, LOC.CrossProduct.Y, LOC.CrossProduct.Z, LOC.DotProduct, LOC.Character->GetLocalRole(), LOC.Character->GetNetMode());
		}
	}
}

void ULockOnComponent::Press()
{
	// If already locked on, unlock.
	if(IsValid(LockedOnCharacter))
	{
		ClearLockedOn();	
		return;
	}

	// Update list of characters that are valid to lock onto.
	GetValidLockOnCharacters();

	// If one or more valid characters exist, lock on to the central one.
	if(ValidLockOnCharacters.Num() > 0)
	{
		SetLockedOn(FindValidCharacterWithHighestDotProduct());
	}
}

void ULockOnComponent::SetLockedOn(ACeremonyCharacter* CharacterToLockOnTo)
{
	LockedOnCharacter = CharacterToLockOnTo;
	LockedOnCharacter->SetOpponentHasLockedOn(true);
	
	SetComponentTickEnabled(true);

	if(IsValid(OwnerCharacter))
	{
		OwnerCharacter->SetIsLockedOn(true);	
	}
}

void ULockOnComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check owner and locked on actor are valid.
	if(!IsValid(LockedOnCharacter) || LockedOnCharacter->GetHealth() == 0.0f || FVector::Dist(OwnerCharacter->GetActorLocation(), LockedOnCharacter->GetActorLocation()) > LockOnSphereRadius)
	{
		ClearLockedOn();
		return;
	}
	
	// Rotate the character YAW to face the locked on actor.
	RotateCharacterYawTowardsTarget(DeltaTime);
	
	// Rotate camera YAW towards locked on actor.
	RotateCameraYawTowardsTarget(DeltaTime);

	// Rotate camera PITCH towards locked on actor.
	RotateCameraPitchTowardsTarget(DeltaTime);
}

void ULockOnComponent::YawInput(const float Value)
{
	// Once the yaw returns below the selection threshold, allow it to select again.
	if(FMath::Abs(Value) < SelectNewTargetThreshold)
	{
		bBlockUntilYawReturn = false;
		return;
	}

	// Yaw is being held outside selection range.
	if(bBlockUntilYawReturn)
	{
		return;
	}
	
	bBlockUntilYawReturn = true;
	
	GetValidLockOnCharacters();

	// No valid ones.
	if(ValidLockOnCharacters.Num() == 0)
	{
		ClearLockedOn();
		return;	
	}

	// Only one, select it.
	if(ValidLockOnCharacters.Num() == 1)
	{
		SetLockedOn(ValidLockOnCharacters[0].Character);

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::YawInput: One one valid pawn to select, %s."), *GetNameSafe(LockedOnCharacter));
		}
		return;
	}
	
	// Multiple; find the currently locked on pawn
	auto LockedOnIndex = -1;
	for(auto Index = 0; Index < ValidLockOnCharacters.Num(); Index++)
	{
		if(ValidLockOnCharacters[Index].Character == LockedOnCharacter)
		{
			LockedOnIndex = Index;
			break;
		}
	}
	
	if(LockedOnIndex == -1)
	{
		// Couldn't find current target in the valid list
		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::YawInput: Couldn't find previously locked on pawn %s."), *GetNameSafe(LockedOnCharacter));
		}

		SetLockedOn(FindValidCharacterWithHighestDotProduct());
		return;
	}
	
	if(Value > 0.0f)
	{
		// Select one to the right
		LockedOnIndex++;
		if(LockedOnIndex == ValidLockOnCharacters.Num())
		{
			// None exist to the right, keep current selection.
			if(bShowDebugMessages)
			{
				UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::YawInput: Couldn't find pawn to the right, keeping selection %s."), *GetNameSafe(LockedOnCharacter));
			}

			return;
		}

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::YawInput: Found pawn to the right %s."), *GetNameSafe(ValidLockOnCharacters[LockedOnIndex].Character));
		}

		SetLockedOn(ValidLockOnCharacters[LockedOnIndex].Character);
	}
	else
	{
		// Select new target to the left of the old target
		LockedOnIndex--;
		if(LockedOnIndex < 0)
		{
			// None exist to the left, keep current selection.
			if(bShowDebugMessages)
			{
				UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::YawInput: Couldn't find pawn to the left, keeping selection %s."), *GetNameSafe(LockedOnCharacter));
			}

			return;
		}

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULockOnComponent::YawInput: Found pawn to the left %s."), *GetNameSafe(ValidLockOnCharacters[LockedOnIndex].Character));
		}
		
		SetLockedOn(ValidLockOnCharacters[LockedOnIndex].Character);
	}
}

#pragma region CameraCorrection

void ULockOnComponent::RotateCameraPitchTowardsTarget(const float DeltaTime) const
{
	const FVector TargetVector = LockedOnCharacter->GetActorLocation() - (OwnerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, LockOnPitchOffset));
	const FVector NormalizedTargetVector = TargetVector.GetSafeNormal();

	const float DesiredPitch = FMath::Atan2(NormalizedTargetVector.Z, FMath::Sqrt(NormalizedTargetVector.X * NormalizedTargetVector.X + NormalizedTargetVector.Y * NormalizedTargetVector.Y));
	
	const FRotator CameraPitchRotation = FRotator(OwnerCharacter->GetControlRotation().Pitch, 0.0f, 0.0f);
	const FVector CameraPitchVector = FRotationMatrix(CameraPitchRotation).GetUnitAxis(EAxis::X);
	const float CameraPitch = FMath::Atan2(CameraPitchVector.Z, FMath::Sqrt(CameraPitchVector.X*CameraPitchVector.X + CameraPitchVector.Y*CameraPitchVector.Y));

	const float Difference = CameraPitch - DesiredPitch;
	if(FMath::IsNearlyZero(Difference))
	{
		return;
	}
		
	const float Step = Difference > 0? FMath::Clamp(DeltaTime * CameraAdjustmentRate, 0.0f, Difference) : FMath::Clamp(DeltaTime * CameraAdjustmentRate * -1.0f, Difference, 0.0f);
	OwnerCharacter->AddControllerPitchInput(Step);
}

void ULockOnComponent::RotateCameraYawTowardsTarget(const float DeltaTime) const
{
	const FRotator CameraYawRotation = FRotator(0.0f, OwnerCharacter->GetControlRotation().Yaw, 0.0f);
	const FVector CameraYawDirection = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);

	const FVector TargetVector = LockedOnCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation();
	const FVector NormalizedTargetVector = FVector(TargetVector.X, TargetVector.Y, 0.0f).GetSafeNormal();
	
	const FVector CrossProductYaw = FVector::CrossProduct(CameraYawDirection, NormalizedTargetVector);
	const float DotProductYaw = FVector::DotProduct(CameraYawDirection, NormalizedTargetVector);

	const float YawAngle = FMath::Acos(DotProductYaw);

	if(CrossProductYaw.IsNearlyZero())
	{
		return;
	}

	const float Step = CrossProductYaw.Z > 0.0f? FMath::Clamp(DeltaTime * CameraAdjustmentRate, 0.0f, YawAngle) : FMath::Clamp(DeltaTime * CameraAdjustmentRate * -1.0f, -YawAngle, 0.0f);
	OwnerCharacter->AddControllerYawInputFromLockOn(Step);
}

#pragma endregion 

#pragma region CharacterYawCorrection

void ULockOnComponent::OnYawCorrectionMontageEnded(UAnimMontage* Montage, const bool bInterrupted)
{
	if(!bInterrupted)
	{
		OwnerCharacter->StopMontageGlobally();
		OwnerCharacter->ClearOnMontageEndedDelegate();
	}

	bPlayingYawCorrectionMontage = false;	
}

void ULockOnComponent::RotateCharacterYawTowardsTarget(const float DeltaTime)
{
	if(!OwnerCharacter->GetAllowMovement())
	{
		return;
	}

	// Make the owner character face the target.
	const FVector TargetVector = LockedOnCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation();
	const FVector NormalizedTargetVector = TargetVector.GetSafeNormal();
	const float TargetYaw = FMath::RadiansToDegrees(FMath::Atan2(NormalizedTargetVector.Y, NormalizedTargetVector.X));

	const float CharacterYaw = OwnerCharacter->GetActorRotation().Yaw;

	float Difference = TargetYaw - CharacterYaw;

	if(FMath::Abs(Difference) < 10.0f)
	{
		return;
	}

	if(Difference > 180.0f)
	{
		Difference -= 360.0f;
	}
	else if(Difference < -180.0f)
	{
		Difference += 360.0f;
	}

	const float Step = Difference > 0.0f? FMath::Clamp(OwnerCharacter->GetCharacterMovement()->RotationRate.Yaw * DeltaTime, 0.0f, Difference)
						: FMath::Clamp(OwnerCharacter->GetCharacterMovement()->RotationRate.Yaw * DeltaTime * -1.0f, Difference, 0.0f);
	
	// If the character is standing still and need to adjust yaw, play correction montage.
	if(!bPlayingYawCorrectionMontage && FMath::IsNearlyZero(OwnerCharacter->GetVelocity().Size(), 1.0f))
	{
		bPlayingYawCorrectionMontage = true;
		OwnerCharacter->PlayMontageGlobally(YawCorrectionMontage);
		OwnerCharacter->SetOnMontageEndedDelegate(this, TEXT("OnYawCorrectionMontageEnded"), YawCorrectionMontage);
	}

	OwnerCharacter->Server_SetActorRotation(OwnerCharacter->GetActorRotation() + FRotator(0.0f, Step, 0.0f));
}

#pragma endregion
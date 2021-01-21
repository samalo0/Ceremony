// Copyright Stephen Maloney 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class ACeremonyCharacter;

/**
 * Structure that holds a pawn that can be possibly locked on to.
 */
USTRUCT(BlueprintType)
struct FValidLockOnCharacter
{

	GENERATED_BODY()

	FValidLockOnCharacter() : Character(nullptr), CrossProduct(0.0f, 0.0f, 0.0f), DotProduct(0.0f) {}
	
	FValidLockOnCharacter(ACeremonyCharacter* InCharacter, const FVector InCrossProduct, const float InDotProduct) : Character(InCharacter), CrossProduct(InCrossProduct), DotProduct(InDotProduct) {}

	UPROPERTY(EditAnywhere)
	ACeremonyCharacter* Character;

	// Value of 0 when two vectors point the same way; goes up to 1 or -1 depending upon side, then falls back off to 0.
	UPROPERTY(EditAnywhere)
	FVector CrossProduct;

	// Value of 1 when two vectors point the same way; falling off until 90 degrees, where it's 0, then it goes negative and increases to -1 when they point opposite.
	UPROPERTY(EditAnywhere)
	float DotProduct;

	FORCEINLINE bool operator <(const FValidLockOnCharacter &Other) const
	{
		return CrossProduct.Z < Other.CrossProduct.Z;
	}
};

class UAnimMontage;

/**
 * Component that adds lock on functionality to a character.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEREMONY_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	ULockOnComponent();
	
	// Triggered when the lock on button is pressed.
	void Press();

	// Tick while locked on in order to update camera rotation and locked on status.
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Yaw input incoming from mouse/controller while locked on. Use to switch targets.
	void YawInput(float Value);

protected:

	void BeginPlay() override;

	// Call internally to clear the locked on character and set necessary settings.
	void ClearLockedOn();
	
	ACeremonyCharacter* FindValidCharacterWithHighestDotProduct();
	
	void GetValidLockOnCharacters();

	// Call internally to set the character that the owner of this component is locking on to.
	void SetLockedOn(ACeremonyCharacter* CharacterToLockOnTo);
	
	// Prevents toggling too quickly between lock on targets.
	bool bBlockUntilYawReturn = false;
	
	// When lock on is attempted, the angle between the camera forward vector and the vector from the character to the target is compared. If they are equal, this value is 1; if they are opposite, -1.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f, ClampMax=1.0f))
	float DotProductRange = 0.5f;

	// The radius of the sphere used to find other characters.
	UPROPERTY(EditDefaultsOnly)
	float LockOnSphereRadius = 1000.0f;
	
	// Reference to the character that is currently locked on to.
	UPROPERTY(Transient)
	ACeremonyCharacter* LockedOnCharacter;
	
	// Cached reference to the owning character.
	UPROPERTY(Transient)
	ACeremonyCharacter* OwnerCharacter;
	
	// Show debug messages.
	UPROPERTY(EditDefaultsOnly)
	bool bShowDebugMessages = false;
	
	// Threshold for input to trigger selecting a new target
	UPROPERTY(EditDefaultsOnly)
	float SelectNewTargetThreshold = 0.5f;

	// References to any valid lock on targets, and the cross product Z value, which shows if the target is left or right and how far.
	UPROPERTY(Transient)
	TArray<FValidLockOnCharacter> ValidLockOnCharacters;

#pragma region CameraCorrection

protected:
	
	void RotateCameraPitchTowardsTarget(float DeltaTime) const;

	void RotateCameraYawTowardsTarget(float DeltaTime) const;

	// The rate that the camera adjusts to follow the target.
	UPROPERTY(EditDefaultsOnly)
	float CameraAdjustmentRate = 50.0f;
	
	// When locked on, a vector from the character to target is created to set camera pitch (looking down/up). This is the offset to place it above your character's head, to look down on the target.
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0f, ClampMax=500.0f))
	float LockOnPitchOffset = 200.0f;
	
#pragma endregion
	
#pragma region CharacterYawCorrection

protected:

	void RotateCharacterYawTowardsTarget(float DeltaTime);
	
	UFUNCTION()
	void OnYawCorrectionMontageEnded(UAnimMontage* Montage, bool bInterrupted);	
	
	// Used to block playing the montage on top of itself.
	bool bPlayingYawCorrectionMontage = false;
	
	// Montage to play when character is standing still but needs to rotate to face target while locked on.
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* YawCorrectionMontage;

#pragma endregion
};

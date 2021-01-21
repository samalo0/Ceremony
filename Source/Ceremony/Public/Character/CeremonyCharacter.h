// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CeremonyAnimNotifyState.h"
#include "CeremonyCharacter.generated.h"

class AEquipmentActor;
class UWidgetComponent;

/**
 * Structure which is used to play cosmetic montages on clients.
 */
USTRUCT(BlueprintType)
struct FCosmeticAnimMontage
{
	GENERATED_BODY()

	void Set(UAnimMontage* NewMontage, const float NewPosition) { Montage = NewMontage; Position = NewPosition; }
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
	float Position = 0.0f;
};


/**
 * Base character class for Ceremony.
 */
UCLASS()
class CEREMONY_API ACeremonyCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ACeremonyCharacter(const class FObjectInitializer& ObjectInitializer);

	// Handle cleanup when the character leaves the game.
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Tick(float DeltaTime) override;
	
protected:

	void BeginPlay() override;
	
#pragma region Audio

protected:

	// Sound to play when an attack connects with another character.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Audio")
	USoundBase* AttackHitSound;
	
	// Sound to play when an attack is blocked.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Audio")
	USoundBase* BlockAttackSound;

	// Sound to play when kick is blocked.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Audio")
	USoundBase* KickBlockedSound;

	// Sound to play when kick interrupts an action or knocks back a character.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Audio")
	USoundBase* KickInterruptSound;
	
	// Sound to play when a parry occurs.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Audio")
	USoundBase* ParrySound;
	
#pragma endregion 
	
#pragma region Client

public:

	// Called when the player number is updated.
	UFUNCTION()
	void OnRep_ClientPlayerNumber() const;

	// Tracks which player number in the world; used to set color.
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ClientPlayerNumber)
	int32 ClientPlayerNumber = 0;
	
protected:

	// Called from the server to trigger the client into back stabbed.
	UFUNCTION(Client, Reliable)
	void Client_BackStabbed();
	void Client_BackStabbed_Implementation();
	
	UFUNCTION(Client, Reliable)
	void Client_DepleteEnduranceCanStagger(float EnduranceToDeplete);
	void Client_DepleteEnduranceCanStagger_Implementation(float EnduranceToDeplete);

	// Called from the server to trigger the client into riposted.
	UFUNCTION(Client, Reliable)
	void Client_Riposted();
	void Client_Riposted_Implementation();
	
	// Called from the server to trigger the client into staggered.
	UFUNCTION(Client, Reliable)
	void Client_Staggered();
	void Client_Staggered_Implementation();
	
	// Called from the server to trigger the client into stunned.
	UFUNCTION(Client, Reliable)
	void Client_Stunned(float InStunTime);
	void Client_Stunned_Implementation(float InStunTime);
	
#pragma endregion
	
#pragma region Components

public:

	// IK component for moving feet and hips.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	class UInverseKinematicsComponent* InverseKinematicsComponent;
	
protected:
	
	// Camera component for the third person camera.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	class UCameraComponent* CameraComponent;
	
	// Debug component for state text.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	class UDebugComponent* DebugComponent;

	// Footstep component for playing footstep sounds.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	class UFootstepComponent* FootstepComponent;
	
	// Collision capsule for the kicking foot.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	UCapsuleComponent* KickCapsuleComponent;

	// Component which enables lock on functionality.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	class ULockOnComponent* LockOnComponent;

	// Widget which displays the lock on symbol when you lock onto an opponent.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	UWidgetComponent* LockOnWidget;
	
	// Component which displays the character health bar and damage when you hit them, as an opponent.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	UWidgetComponent*  OpponentWidget;
	
	// Spring arm component for attaching the third person camera.
	UPROPERTY(VisibleAnywhere, Category = "CeremonyCharacter | Components")
	class USpringArmComponent* SpringArmComponent;

#pragma endregion
	
#pragma region Equipment

public:

	void CancelBlocking();

	// Check for transitioning from one attack into another, triggered by animation notify.
	void CheckForAttackTransition(bool bIsRightHanded) const;

	bool GetCanAttack() const;
	
	bool GetCanBlock() const;
	
	FORCEINLINE bool GetIsAttacking() const { return bIsAttacking; }

	FORCEINLINE bool GetIsBlocking() const { return bIsBlocking; }

	FORCEINLINE bool GetIsParrying() const { return bIsParrying; }

	FORCEINLINE bool GetIsShieldLeftHanded() const { return bIsShieldLeftHanded; }
	
	FORCEINLINE bool GetParryCanStagger() const { return bParryCanStagger; }
	
	// Called from animation notify state when a montage sets a weapon to active or inactive.
	void SetAttackCanDamage( bool bRightHand, bool bIsActive) const;

	void SetIsAiming(bool bAiming);
	
	void SetIsAttacking(bool bAttacking);

	void SetIsBlocking(bool bBlocking, bool bIsLeftHanded = true);

	void SetIsParrying(bool bParry);

	void SetParryCanStagger(bool bCanStagger);
	
protected:
	
	void LeftHandPress1();
	void LeftHandPress2();
	void LeftHandRelease1();
	void LeftHandRelease2();
	
	void RightHandPress1();
	void RightHandPress2();
	void RightHandRelease1();
	void RightHandRelease2();

	// State set when attack animations are being played.
	bool bIsAttacking = false;

	// State set while aiming projectile animations are being played.
	bool bIsAiming = false;
	
	// State set when block animations are being played.
	bool bIsBlocking = false;
	
	bool bIsShieldLeftHanded = true;
	
	// Reference to the item equipped in the left hand.
	UPROPERTY(Transient, Replicated)
	AEquipmentActor* LeftHandEquipment;

	// Class to spawn for the default left hand equipment.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Equipment")
	TSubclassOf<AEquipmentActor> LeftHandEquipmentDefaultClass;

	// Socket with which to attach right hand equipment.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Equipment")
	FName LeftHandSocketName = TEXT("LeftHandSocket");

	// Keep track of if a left press is held down.
	bool bLeftHandPress1HeldDown = false;
	bool bLeftHandPress2HeldDown = false;

	bool bIsParrying = false;

	bool bParryCanStagger = false;
	
	// Reference to the item equipped in the right hand.
	UPROPERTY(Transient, Replicated)
	AEquipmentActor* RightHandEquipment;

	// Class to spawn for the default right hand equipment.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Equipment")
	TSubclassOf<AEquipmentActor> RightHandEquipmentDefaultClass;

	// Socket with which to attach right hand equipment.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Equipment")
	FName RightHandSocketName = TEXT("RightHandSocket");

	// Keep track of if a left press is held down.
	bool bRightHandPress1HeldDown = false;
	bool bRightHandPress2HeldDown = false;
	
#pragma endregion

#pragma region Gameplay

public:

	// Cancels all actions the character could be currently taking.
	void CancelActions();
	
	// Check and see if a button is being held down that triggers an action after another finishes.
	void CheckForResumingAction();

	void DepleteEndurance(float EnduranceChange);
	
	FORCEINLINE bool GetAllowEnduranceRecovery() const { return bAllowEnduranceRecovery; }

	// Returns if the character is free to perform actions that are singular; attacking, rolling, jumping, etc.
	bool GetCanPerformStandardAction() const;
	
	FORCEINLINE float GetEndurance() const { return Endurance; }

	FORCEINLINE float GetHealth() const { return Health; }

	FORCEINLINE bool GetIsInvincible() const { return bIsInvincible; }

	FORCEINLINE bool GetIsStaggered() const { return bIsStaggered; }
	
	FORCEINLINE bool GetIsStunned() const { return bIsStunned; }
	
	bool IsShowingDebugCollision() const;

	// Called via notify to check the material below the character and play the appropriate footstep sound.
	void PlayFootstepSound(bool bRightFoot);

	// Play a sound at the actor's location.
	void PlaySound(USoundBase* Sound) const;
	
	void SetAllowEnduranceRecovery(bool bAllowRecovery);

	void SetIsInvincible(bool bInvincible);

	void SetIsStaggered(bool bStaggered);
	
	void SetIsStunned(bool bStunned);

	// Call when a locally controlled opponent has locked on to this character (or released lock on).
	void SetOpponentHasLockedOn(bool bHasLockedOn) const;
	
protected:
	
	UFUNCTION()
	void OnRep_Health() const;

	// If the character is in a state that allows endurance to be recovered.
	bool bAllowEnduranceRecovery = true;
	
	// Boolean that keeps track of if movement is allowed.
	bool bAllowMovement = true;

	// Endurance recovery rate per second when aiming.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	float AimingEnduranceRecoveryPerSecond = 10.0f;
	
	// Endurance recovery rate per second when blocking.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	float BlockingEnduranceRecoveryPerSecond = 10.0f;
	
	// Character endurance; at 0 (or negative values) they cannot perform actions.
	UPROPERTY(Transient)
	float Endurance;

	// Maximum amount of character endurance, and the value at which you start.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	float EnduranceMaximum = 100.0f;
	
	// Endurance recovery rate per second when walking/standing.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	float EnduranceRecoveryPerSecond = 40.0f;
	
	// Character health; at 0, they die.
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Health)
	float Health;

	// Maximum amount of character health, and the value at which you start.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	float HealthMaximum = 100.0f;

	bool bIsInvincible = false;

	// Whether the character is staggered and able to be riposted. This must be replicated.
	UPROPERTY(Transient, Replicated)
	bool bIsStaggered = false;
	
	// Whether the character is stunned and unable to take actions or not.
	bool bIsStunned = false;

	// The amount of time to remain in stagger before being released automatically.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	float StaggerTime = 2.0f;
	
	// Timer to count down before stagger is released.
	float StaggerTimer = 0.0f;
	
	// The number of times hit while stunned, to trigger early stun exit.
	int32 StunCount = 0;

	// The maximum times the character can be hit while stunned before being instantly un-stunned.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Gameplay")
	int32 StunCountMaximum = 1;

	// Timer to count down time before stun is released.
	float StunTimer = 0.0f;
	
#pragma endregion 

#pragma region Kick

public:
	
	FORCEINLINE bool GetIsKicking() const { return bIsKicking; }

	void SetIsKicking(bool bKick);
	
	void SetKickCanDamage(bool bCanDamage) const;
	
protected:

	void Kick();

	UFUNCTION()
	void OnKickComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnKickMontageComplete(UAnimMontage* Montage, const bool bInterrupted);

	bool bIsKicking = false;
	
	// Array to keep track of who's been kicked in a single kick. Prevents kicking the same person more than once with one kick.
	UPROPERTY(Transient)
	TArray<AActor*> KickedActors;

	// The endurance damage done
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Kick")
	float KickEnduranceDamage = 90.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Kick")
	float KickEnduranceConsumption = 20.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Kick")
	UAnimMontage* KickMontage;

	// Queues up a kick on resume action if pressed while another action is in process.
	bool bKickOnResumeAction = false;

	// The amount of time to trigger stun with a kick, when it hits a non-blocking opponent.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Kick")
	float KickStunTime = 0.1f;
	
#pragma endregion
	
#pragma region Montage

public:

	// Clears all delegates that would be called when a montage playback ends.
	void ClearOnMontageEndedDelegate() const;

	// Play a montage locally on the client, and replicate it via the server to other clients to play with CosmeticAnimMontage.
	void PlayMontageGlobally(UAnimMontage* MontageToPlay, FName JumpToSection = NAME_None);	

	// Sets the function to call when a playing montage has finished playing or is interrupted.
	void SetOnMontageEndedDelegate(UObject* UserObject, FName FunctionName, UAnimMontage* ActiveMontage) const;

	// Stop the animation montage playing on the character locally, and tell the server to clear the cosmetic montage playback.
	void StopMontageGlobally();
	
protected:

	UFUNCTION()
	void OnBackStabOrRiposteMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	// Triggered when the server changes this value on clients.
	UFUNCTION()
	void OnRepCosmeticAnimMontage() const;
	
	// Internal function for playing a montage locally.
	float PlayMontage(UAnimMontage* MontageToPlay, FName JumpToSection) const;

	// Montage to play when the character is back stabbed.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Montage")
	UAnimMontage* BackStabMontage;
	
	// When one client plays a montage, the server updates this value on the other clients to trigger that montage to play remotely.
	UPROPERTY(Transient, ReplicatedUsing=OnRepCosmeticAnimMontage)
	FCosmeticAnimMontage CosmeticAnimMontage;
	
	// The amount of time to blend animations after a montage has been interrupted or stopped.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Montage")
	float MontageBlendOutTime = 0.25;

	// Montage to play when the character is riposted.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Montage")
	UAnimMontage* RiposteMontage;
	
	// Montage to play when the character is staggered (unable to take any actions and ripostable).
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Montage")
	UAnimMontage* StaggerMontage;

	// Montage to play when the character is stunned (unable to take any actions).
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Montage")
	UAnimMontage* StunMontage;
	
#pragma endregion 
	
#pragma region Movement

public:

	// Override to add lock on functionality.
	void AddControllerPitchInput(float Val) override;
	
	// Override to add lock on functionality.
	void AddControllerYawInput(float Val) override;

	// Allow yaw input to the camera from the locked on component.
	void AddControllerYawInputFromLockOn(float Val);
	
	FORCEINLINE bool GetAllowMovement() const { return bAllowMovement; }
	
	void GetForcedMovement(bool& bOutForcedMovement, float& OutForcedMovementRate, FVector& OutForcedMovementDirection) const;

	FORCEINLINE bool GetIsRunning() const { return bIsRunning; }

	FORCEINLINE bool GetIsLockedOn() const { return bIsLockedOn; }

	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }

	FORCEINLINE bool IsMeleeLocomotion() const { return bMeleeLocomotion; }
	
	// Called on all (non-locally controlled) clients when a lock on change occurs.
	UFUNCTION()
	void OnRep_IsLockedOn() const;
	
	void SetAllowMovement(bool bAllow);
	
	// Called from animation notify state, to trigger the character to move while playing animation montages.
	void SetAnimMovement(bool bInForcedMovement, float InForcedMovementRate = 0.0f, EMovementType UnlockedType = EMovementType::ForcedForwardOrBack, EMovementType LockedType = EMovementType::ForcedForwardOrBack);

	// Called from the locked on component to enable/disable lock.
	void SetIsLockedOn(bool bLocked);
	
	void SetIsRunning(bool bRun);
	
protected:

	void Jump() override;

	void LockOn();
	
	// Input function called when pressing forward/back on the movement stick.
	void MoveForward(float AxisValue);
	float MoveForwardLastValue = 0.0f;
	
	// Input function called when pressing left/right on the movement stick.
	void MoveRight(float AxisValue);
	float MoveRightLastValue = 0.0f;

	void RunPress();

	void RunRelease();
	
	// Keeps track if movement is forced by playing back an animation.
	bool bForcedMovement = false;

	// The direction to force movement.
	FVector ForcedMovementDirection;
	
	// The rate that movement is forced while playing animation.
	float ForcedMovementRate = 0.0f;

	// Whether the character is currently locked on to another character.
	UPROPERTY(Transient, ReplicatedUsing=OnRep_IsLockedOn)
	bool bIsLockedOn;
	
	// Whether the character is currently running.
	UPROPERTY(Transient, Replicated)
	bool bIsRunning;

	// The amount of endurance to consume when jumping.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Movement", meta=(ClampMin=0.0f))
	float JumpEnduranceConsumption = 40.0f;

	UPROPERTY(Transient, Replicated)
	bool bMeleeLocomotion = true;
	
	// The amount of endurance to consume per second while running.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Movement", meta=(ClampMin=0.0f))
	float RunEnduranceCostPerSecond = 20.0f;

	// Keeps track of if the run button is held down to resume running after actions.
	bool bRunHeldDown = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Movement", meta=(ClampMin=20.0f))
	float RunSpeed = 600.0f;
	
	// If while running you run the endurance to zero, the endurance is set to this value.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Movement", meta=(ClampMax=0.0f))
	float RunToZeroEndurancePenalty = -30.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Movement", meta=(ClampMin=10.0f))
	float WalkSpeed = 400.0f;
	
#pragma endregion

#pragma region Multicast

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound(USoundBase* Sound);
	void Multicast_PlaySound_Implementation(USoundBase* Sound);
	
	// Called from the server to tell all clients a particular character has died.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_KillCharacter(ACeremonyCharacter* CharacterToKill);
	void Multicast_KillCharacter_Implementation(ACeremonyCharacter* CharacterToKill);

	// Called from the server to force an actor rotation while locked on.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetActorRotation(FRotator Rotation);
	void Multicast_SetActorRotation_Implementation(FRotator Rotation);
	
	// Called from the server to tell all simulated proxies to update their damage indicator.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetOpponentWidgetDamage(float Damage);
	void Multicast_SetOpponentWidgetDamage_Implementation(float Damage);
	
#pragma endregion

#pragma region Roll

public:

	FORCEINLINE bool GetIsRolling() const { return bIsRolling; }

	void SetIsRolling(bool bRoll);
	
protected:
	
	UFUNCTION()
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void Roll();
	
	// Whether the character is currently rolling.
	UPROPERTY(Transient)
	bool bIsRolling;

	// If roll is pressed while locked in another action, start up the roll when the action completes.
	bool bRollOnResumeAction = false;
	
	// The amount of endurance to consume when rolling.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Roll", meta=(ClampMin=0.0f))
	float RollEnduranceConsumption = 20.0f;

	// Montage to play when the character is rolling.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Roll")
	UAnimMontage* RollMontage;
	
	// The time frame that you have to press/release the run button to trigger a roll.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Roll", meta=(ClampMin=0.01f, ClampMax=1.0f))
	float RollPressReleaseTime = 0.2f;

	// The last time run was pressed. Used to determine if a roll should happen.
	float RollTimeStamp = 0.0f;
	
#pragma endregion
	
#pragma region Server

public:

	// A character who plays montages triggers the server to push a cosmetic variable change on the other clients, to see the montage as it's played. The variable must change for this to occur, therefore
	// at the end of each montage playback, this needs to be reset to nullptr/0.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayCosmeticAnimMontage(UAnimMontage* MontageToPlay, float Position);
	void Server_PlayCosmeticAnimMontage_Implementation(UAnimMontage* MontageToPlay, float Position);
	bool Server_PlayCosmeticAnimMontage_Validate(UAnimMontage* MontageToPlay, float Position) { return true; }	

	// Play a sound at the player's location on all clients.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlaySound(USoundBase* Sound);
	void Server_PlaySound_Implementation(USoundBase* Sound) { Multicast_PlaySound(Sound); }
	bool Server_PlaySound_Validate(USoundBase* Sound) { return true; }
		
	// Rotate the actor on the server, which will replicate to all clients.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetActorRotation(FRotator NewRotation);
	void Server_SetActorRotation_Implementation(FRotator NewRotation);
	bool Server_SetActorRotation_Validate(FRotator NewRotation) { return true; }
	
	// The server must know the character is blocking during ServerVerifyOverlapForDamage.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsBlocking(bool bBlocking);
	void Server_SetIsBlocking_Implementation(const bool bBlocking) { bIsBlocking = bBlocking; }
	bool Server_SetIsBlocking_Validate(bool bBlocking) { return true; }

	// The server must know the character is invincible during ServerVerifyOverlapForDamage.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsInvincible(bool bInvincible);
	void Server_SetIsInvincible_Implementation(const bool bInvincible) { bIsInvincible = bInvincible; }
	bool Server_SetIsInvincible_Validate(bool bInvincible) { return true; }

	// The server must know if the character is locked on in order to animate properly; while locked on, the character strafes and faces the target.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsLockedOn(bool bLocked);
	void Server_SetIsLockedOn_Implementation(const bool bLocked) { SetIsLockedOn(bLocked); }
	bool Server_SetIsLockedOn_Validate(bool bLocked) { return true; }
	
	// The server must know the character is running in order to adjust location at the right rate. Otherwise the character will rubber band as the client disagrees with the server.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsRunning(bool bInIsRunning);
	void Server_SetIsRunning_Implementation(const bool bInIsRunning) { bIsRunning = bInIsRunning; }
	bool Server_SetIsRunning_Validate(bool bInIsRunning) { return true; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsStaggered(bool bStagger);
	void Server_SetIsStaggered_Implementation(const bool bStagger) { bIsStaggered = bStagger; }
	bool Server_SetIsStaggered_Validate(bool bStagger) { return true; }
	
	// The server must know that the character is in active parry frames, to parry other characters in ServerVerifyOverlapForDamage.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetParryCanStagger(bool bCanStagger);
	void Server_SetParryCanStagger_Implementation(const bool bCanStagger) { bParryCanStagger = bCanStagger; }
	bool Server_SetParryCanStagger_Validate(bool bCanStagger) { return true; }
	
	// When a back stab connects on a client, verify on the server.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_VerifyBackStab(ACeremonyCharacter* CharacterHit, float Damage);
	void Server_VerifyBackStab_Implementation(ACeremonyCharacter* CharacterHit, float Damage);
	bool Server_VerifyBackStab_Validate(ACeremonyCharacter* CharacterHit, float Damage) { return true; };
	
	// When an attack connects on a client, a sweep is done on the server to verify that a hit was made.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_VerifyOverlapForDamage(ACeremonyCharacter* CharacterHit, FVector_NetQuantize ImpactPoint, FVector_NetQuantize DamageEnduranceDamageStunTime, uint8 DamageType);
	void Server_VerifyOverlapForDamage_Implementation(ACeremonyCharacter* CharacterHit, FVector_NetQuantize ImpactPoint, FVector_NetQuantize DamageEnduranceDamageStunTime, uint8 IntDamageType);
	bool Server_VerifyOverlapForDamage_Validate(ACeremonyCharacter* CharacterHit, FVector_NetQuantize ImpactPoint, FVector_NetQuantize DamageEnduranceDamageStunTime, uint8 DamageType);

	// When a riposte connects on a client, verify on the server.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_VerifyRiposte(ACeremonyCharacter* CharacterHit, float Damage);
	void Server_VerifyRiposte_Implementation(ACeremonyCharacter* CharacterHit, float Damage);
	bool Server_VerifyRiposte_Validate(ACeremonyCharacter* CharacterHit, float Damage) { return true; };
	
protected:

	void Server_HelperKillCharacter(ACeremonyCharacter* Character);
	
	// When a back stab happens locally, it's verified on the server with a dot product and distance limit. This shouldn't be less than the local setting.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Server", meta=(ClampMin=1.0f))
	float ServerBackStabMaxDistance = 100.0f;

	// When a back stab happens locally, it's verified on the server with a dot product and distance limit. This shouldn't be greater than the local setting.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Server", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float ServerBackStabMinDotProduct = 0.8f;
	
	// When a riposte happens locally, it's verified on the server with a dot product and distance limit. This shouldn't be less than the local setting.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Server", meta=(ClampMin=1.0f))
	float ServerRiposteMaxDistance = 100.0f;

	// When a riposte happens locally, it's verified on the server with a dot product and distance limit. This shouldn't be greater than the local setting.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Server", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float ServerRiposteMaxDotProduct = -0.8f;
	
	// When a character hits locally, a sphere overlap at that location is done on the server to verify the character is there.
	UPROPERTY(EditDefaultsOnly, Category = "CeremonyCharacter | Server", meta=(ClampMin=1.0f))
	float ServerVerifyOverlapsSphereRadius = 20.0f;
	
#pragma endregion 
	
};

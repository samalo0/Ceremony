// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipmentStructs.h"
#include "EquipmentActor.generated.h"

/**
 * Base class for equipment - objects that can be held in hands and interacted with through press, release, and resume.
 */
UCLASS()
class CEREMONY_API AEquipmentActor : public AActor
{

	GENERATED_BODY()
	
public:	

	AEquipmentActor();

	virtual void CancelActions() { UE_LOG(LogTemp, Warning, TEXT("CancelActions not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }

	virtual void CheckForAttackTransition() { UE_LOG(LogTemp, Warning, TEXT("CheckForAttackTransition not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner()));}

	EEquipmentStates GetEquipmentState() const { return EquipmentState; }

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Press1() { UE_LOG(LogTemp, Warning, TEXT("Press1 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void Press2() { UE_LOG(LogTemp, Warning, TEXT("Press2 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void Release1() { UE_LOG(LogTemp, Warning, TEXT("Release1 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void Release2() { UE_LOG(LogTemp, Warning, TEXT("Release2 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void Resume1(bool bHeldDown) { UE_LOG(LogTemp, Warning, TEXT("Resume1 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void Resume2(bool bHeldDown) { UE_LOG(LogTemp, Warning, TEXT("Resume2 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }

	virtual void TwoHandPress1() { UE_LOG(LogTemp, Warning, TEXT("TwoHandPress1 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void TwoHandPress2() { UE_LOG(LogTemp, Warning, TEXT("TwoHandPress2 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void TwoHandRelease1() { UE_LOG(LogTemp, Warning, TEXT("TwoHandRelease1 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void TwoHandRelease2() { UE_LOG(LogTemp, Warning, TEXT("TwoHandRelease2 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void TwoHandResume1(bool bHeldDown) { UE_LOG(LogTemp, Warning, TEXT("TwoHandResume1 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	virtual void TwoHandResume2(bool bHeldDown) { UE_LOG(LogTemp, Warning, TEXT("TwoHandResume2 not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }
	
	virtual void SetAttackCanDamage(bool bCanDamage) { UE_LOG(LogTemp, Warning, TEXT("SetAttackCanDamage not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); }

	virtual void ShowCollision(bool bShow) { UE_LOG(LogTemp, Warning, TEXT("ShowCollision not overidden on %s, character %s."), *GetNameSafe(this), *GetNameSafe(GetOwner())); } 
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetEquipmentState(const EEquipmentStates NewState);
	void ServerSetEquipmentState_Implementation(const EEquipmentStates NewState) { EquipmentState = NewState; }
	bool ServerSetEquipmentState_Validate(const EEquipmentStates NewState) { return true; }

	UPROPERTY(EditDefaultsOnly)
	bool bMeleeLocomotion = true;
	
protected:

	void BeginPlay() override;
	
	// The equipment state, replicated from server to clients.
	UPROPERTY(Transient, Replicated)
	EEquipmentStates EquipmentState;

	// Owner reference.
	UPROPERTY(Transient)
	class ACeremonyCharacter* OwnerCharacter;

};

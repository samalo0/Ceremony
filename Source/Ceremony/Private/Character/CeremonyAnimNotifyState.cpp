// Copyright Stephen Maloney 2020

#include "Character/CeremonyAnimNotifyState.h"

#include "Character/CeremonyCharacter.h"
#include "Components/SkeletalMeshComponent.h"

#if WITH_EDITOR
bool UCeremonyAnimNotifyState::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperValue = Super::CanEditChange(InProperty);

	if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotifyState, MovementRate) ||
		InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotifyState, UnlockedMovementType) ||
		InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotifyState, LockedMovementType))
	{
		return NotifyStateType == ECeremonyAnimNotifyStateType::Movement;
	}

	if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotifyState, bAttackIsRightHanded))
	{
		return NotifyStateType == ECeremonyAnimNotifyStateType::Attack;
	}
	
	return SuperValue;
}
#endif

FString UCeremonyAnimNotifyState::GetNotifyName_Implementation() const
{
	switch(NotifyStateType)
	{
	case ECeremonyAnimNotifyStateType::Attack:
		return "Attack";
	case ECeremonyAnimNotifyStateType::Invincibility:
		return "Invincibility";
	case ECeremonyAnimNotifyStateType::Kick:
		return "Kick";
	case ECeremonyAnimNotifyStateType::Movement:
		return "Movement";		
	case ECeremonyAnimNotifyStateType::Parry:
		return "Parry";
	}

	return "Unknown";
}

void UCeremonyAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	// Only want to trigger notifies on controlled clients - simulated clients don't receive notifies.
	ACeremonyCharacter* OwnerCharacter = Cast<ACeremonyCharacter>(MeshComp->GetOwner());
	if(!IsValid(OwnerCharacter) || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	
	switch(NotifyStateType)
	{
	case ECeremonyAnimNotifyStateType::Attack:
		OwnerCharacter->SetAttackCanDamage(bAttackIsRightHanded, true);
		break;
	case ECeremonyAnimNotifyStateType::Invincibility:
		OwnerCharacter->SetIsInvincible(true);
		break;
	case ECeremonyAnimNotifyStateType::Kick:
		OwnerCharacter->SetKickCanDamage(true);
		break;
	case ECeremonyAnimNotifyStateType::Movement:
		OwnerCharacter->SetAnimMovement(true, MovementRate, UnlockedMovementType, LockedMovementType);
		break;
	case ECeremonyAnimNotifyStateType::Parry:
		OwnerCharacter->SetParryCanStagger(true);
		break;
	}
}

void UCeremonyAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	// Only want to trigger notifies on locally controlled clients.
	ACeremonyCharacter* OwnerCharacter = Cast<ACeremonyCharacter>(MeshComp->GetOwner());
	if(!IsValid(OwnerCharacter) || !OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	switch(NotifyStateType)
	{
	case ECeremonyAnimNotifyStateType::Attack:
		OwnerCharacter->SetAttackCanDamage(bAttackIsRightHanded, false);
		break;
	case ECeremonyAnimNotifyStateType::Invincibility:
		OwnerCharacter->SetIsInvincible(false);
		break;
	case ECeremonyAnimNotifyStateType::Kick:
		OwnerCharacter->SetKickCanDamage(false);
		break;
	case ECeremonyAnimNotifyStateType::Movement:
		OwnerCharacter->SetAnimMovement(false);
		break;
	case ECeremonyAnimNotifyStateType::Parry:
		OwnerCharacter->SetParryCanStagger(false);
		break;
	}
}


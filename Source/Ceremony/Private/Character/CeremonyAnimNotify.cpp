// Copyright Stephen Maloney 2020

#include "Character/CeremonyAnimNotify.h"

#include "Character/CeremonyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"

UCeremonyAnimNotify::UCeremonyAnimNotify()
{
	NotifyColor = FColor(255, 0, 0, 255);
}

bool UCeremonyAnimNotify::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperValue = Super::CanEditChange(InProperty);

	if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotify, bCanTransitionRightHand))
	{
		return NotifyType == ECeremonyAnimNotifyType::CanTransitionToNextAttack;
	}

	if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotify, Sound) || InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotify, bPlaySoundMulticast))
	{
		return NotifyType == ECeremonyAnimNotifyType::PlaySound;
	}

	if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCeremonyAnimNotify, bRightFoot))
	{
		return NotifyType == ECeremonyAnimNotifyType::PlayFootstepSound;
	}
	
	return SuperValue;
}

FString UCeremonyAnimNotify::GetNotifyName_Implementation() const
{
	switch(NotifyType)
	{
	case ECeremonyAnimNotifyType::CanTransitionToNextAttack:
		return "CanTransition";
	case ECeremonyAnimNotifyType::PlaySound:
		if(IsValid(Sound))
		{
			return Sound->GetName();	
		}
	case ECeremonyAnimNotifyType::PlayFootstepSound:
		if(bRightFoot)
		{
			return "RightFootstep";
		}
		return "LeftFootstep";
	}

	return Super::GetNotifyName_Implementation();
}

void UCeremonyAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	ACeremonyCharacter* OwnerCharacter = Cast<ACeremonyCharacter>(MeshComp->GetOwner());
	if(!IsValid(OwnerCharacter))
	{
		// Special case - play sound in editor
		if(NotifyType == ECeremonyAnimNotifyType::PlaySound)
		{
			UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), Sound, MeshComp->GetComponentLocation(), 1.0f, 1.0f);
		}
		return;
	}

	// Only locally controlled characters receive notifies. These don't occur on a dedicated server or simulated clients.
	if(!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	
	switch(NotifyType)
	{
	case ECeremonyAnimNotifyType::CanTransitionToNextAttack:
		OwnerCharacter->CheckForAttackTransition(bCanTransitionRightHand);
		break;
	case ECeremonyAnimNotifyType::PlaySound:
		if(bPlaySoundMulticast)
		{
			OwnerCharacter->Server_PlaySound(Sound);
		}
		else
		{
			OwnerCharacter->PlaySound(Sound);
		}
		break;
	case ECeremonyAnimNotifyType::PlayFootstepSound:
		OwnerCharacter->PlayFootstepSound(bRightFoot);
		break;
	}
}


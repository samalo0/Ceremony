// Copyright 2020 Stephen Maloney

#include "Equipment/ShieldActor.h"

#include "Character/CeremonyCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

AShieldActor::AShieldActor()
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->ArrowSize = 0.25f;
	SetRootComponent(ArrowComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->SetCollisionProfileName("NoCollision");
}

void AShieldActor::CancelActions()
{
	bParryOnResume = false;

	if(OwnerCharacter->GetIsBlocking())
	{
		OwnerCharacter->SetIsBlocking(false, GetEquipmentState() == EEquipmentStates::EquippedLeftHand);
		OwnerCharacter->StopMontageGlobally();
	}

	if(OwnerCharacter->GetIsParrying())
	{
		OwnerCharacter->SetIsParrying(false);
		OwnerCharacter->ClearOnMontageEndedDelegate();
		OwnerCharacter->StopMontageGlobally();
		OwnerCharacter->SetAllowMovement(true);
		OwnerCharacter->SetAllowEnduranceRecovery(true);
	}
}

void AShieldActor::GetDamageAfterAbsorption(const float DamageIn, const EDamageTypes DamageType, const float EnduranceDamageIn, float& DamageOut, float& EnduranceDamageOut) const
{
	switch(DamageType)
	{
	case EDamageTypes::Bludgeon:
	case EDamageTypes::Pierce:
	case EDamageTypes::Slash:
	case EDamageTypes::Kick:
		DamageOut = DamageIn * (1.0f - ShieldBlockProps.PhysicalDefense);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Shield %s - Unknown damage type, not absorbing."), *GetNameSafe(this));
		DamageOut = DamageIn;
		break;
	}
	
	EnduranceDamageOut = EnduranceDamageIn * (1.0f - ShieldBlockProps.Stability);
}

#pragma region Press1

void AShieldActor::OnImpactMontageEnded(UAnimMontage* Montage, bool bInterrupted) const
{
	OwnerCharacter->ClearOnMontageEndedDelegate();

	if(OwnerCharacter->GetIsBlocking())
	{
		OwnerCharacter->PlayMontageGlobally(BlockingMontage, BlockMontageIdleSectionName);
	}
}

void AShieldActor::Press1()
{
	if(OwnerCharacter->GetCanBlock())
	{
		OwnerCharacter->SetIsBlocking(true, GetEquipmentState() == EEquipmentStates::EquippedLeftHand);
		OwnerCharacter->PlayMontageGlobally(BlockingMontage);
	}
}

void AShieldActor::Release1()
{
	if(OwnerCharacter->GetIsBlocking())
	{
		OwnerCharacter->SetIsBlocking(false, GetEquipmentState() == EEquipmentStates::EquippedLeftHand);
		OwnerCharacter->PlayMontageGlobally(BlockingMontage, BlockMontageExitSectionName);
	}
}

void AShieldActor::Resume1(const bool bHeldDown)
{
	if(bHeldDown)
	{
		Press1();	
	}
}

void AShieldActor::ShowBlockImpact()
{
	OwnerCharacter->PlayMontageGlobally(ImpactMontage);
	OwnerCharacter->SetOnMontageEndedDelegate(this, "OnImpactMontageEnded", ImpactMontage);
}

#pragma endregion

#pragma region Press2

void AShieldActor::Press2()
{
	// A parry requires similar state to rolling/kicking
	if(OwnerCharacter->GetCanPerformStandardAction())
	{
		OwnerCharacter->DepleteEndurance(ParryEnduranceConsumption);
		OwnerCharacter->SetAllowMovement(false);
		OwnerCharacter->SetAllowEnduranceRecovery(false);
		OwnerCharacter->SetIsParrying(true);
		OwnerCharacter->PlayMontageGlobally(ParryMontage);
		OwnerCharacter->SetOnMontageEndedDelegate(this, "OnParryMontageEnded", ParryMontage);
	}
	else
	{
		bParryOnResume = true;
	}
}

void AShieldActor::Resume2(bool bHeldDown)
{
	if(bParryOnResume)
	{
		bParryOnResume = false;
		Press2();
	}
}

void AShieldActor::OnParryMontageEnded(UAnimMontage* Montage, const bool bInterrupted) const
{
	OwnerCharacter->ClearOnMontageEndedDelegate();
	OwnerCharacter->StopMontageGlobally();
	OwnerCharacter->SetIsParrying(false);
	OwnerCharacter->SetAllowMovement(true);
	OwnerCharacter->SetAllowEnduranceRecovery(true);
}

#pragma endregion
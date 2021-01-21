// Copyright 2020 Stephen Maloney

#include "Character/DebugComponent.h"

#include "Components/CapsuleComponent.h"
#include "Character/CeremonyCharacter.h"
#include "Core/CeremonyPlayerController.h"
#include "Character/DebugUserWidget.h"
#include "Components/InputComponent.h"
#include "Character/InverseKinematicsComponent.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

UDebugComponent::UDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACeremonyCharacter>(GetOwner());
	check(IsValid(OwnerCharacter));

	if(!OwnerCharacter->IsLocallyControlled())
	{
		// Non-fully functioning characters will not use the debug component.
		return;
	}
	
	ACeremonyPlayerController* CeremonyController = Cast<ACeremonyPlayerController>(OwnerCharacter->GetController());
	check(IsValid(CeremonyController));
	
	DebugUserWidget =  CeremonyController->DebugWidget;
	check(IsValid(DebugUserWidget));
	
	DebugUserWidget->SetNetModeText(OwnerCharacter->GetNetMode());
	DebugUserWidget->SetNetRoleText(OwnerCharacter->GetLocalRole());
}

void UDebugComponent::BindActions(UInputComponent* InputComponent)
{
	InputComponent->BindAction("DebugShowCollision", IE_Pressed, this, &UDebugComponent::ToggleShowCollision);
	InputComponent->BindAction("DebugShowIKTraces", IE_Pressed, this, &UDebugComponent::ToggleShowIKTraces);
	InputComponent->BindAction("DebugSlowMotion", IE_Pressed, this, &UDebugComponent::ToggleSlowMotion);
	
	InputComponent->BindAction("DebugToggle", IE_Pressed, this, &UDebugComponent::DebugToggle);
}

void UDebugComponent::DebugToggle()
{
	if(IsValid(DebugUserWidget))
	{
		if(DebugUserWidget->GetVisibility() == ESlateVisibility::Hidden)
		{
			DebugUserWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			DebugUserWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UDebugComponent::SetAttackCanDamageText(const bool bCanDamage) const
{
	DebugUserWidget->SetAttackCanDamageText(bCanDamage);
}

void UDebugComponent::SetKickCanDamageText(const bool bCanDamage) const
{
	DebugUserWidget->SetKickCanDamageText(bCanDamage);
}

void UDebugComponent::ToggleShowCollision()
{
	if(IsValid(OwnerCharacter) && IsValid(DebugUserWidget))
	{
		bEnableCollisionDebug = !bEnableCollisionDebug;

		if(bEnableCollisionDebug)
		{
			OwnerCharacter->GetCapsuleComponent()->SetHiddenInGame(false);
			DebugUserWidget->SetShowCollisionTextActive(true);
		}
		else
		{
			OwnerCharacter->GetCapsuleComponent()->SetHiddenInGame(true);
			DebugUserWidget->SetShowCollisionTextActive(false);
		}
	}
}

void UDebugComponent::ToggleShowIKTraces()
{
	if(!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugService: Can't get OwnerCharacter."));
		return;
	}

	if(!IsValid(DebugUserWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugService: Can't get DebugWidget."));
		return;
	}

	if(OwnerCharacter->InverseKinematicsComponent->GetShowDebugTraces())
	{
		OwnerCharacter->InverseKinematicsComponent->SetShowDebugTraces(false);
		DebugUserWidget->SetShowIKTraceTextActive(false);
	}
	else
	{
		OwnerCharacter->InverseKinematicsComponent->SetShowDebugTraces(true);
		DebugUserWidget->SetShowIKTraceTextActive(true);
	}
}

void UDebugComponent::ToggleSlowMotion()
{
	if(!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugService: Can't get OwnerCharacter."));
		return;
	}

	UWorld* World = OwnerCharacter->GetWorld();
	if(!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugService: Can't get World."));
		return;
	}

	if(!IsValid(DebugUserWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugService: Can't get Owner."));
		return;
	}
	
	if(World->GetWorldSettings()->GetEffectiveTimeDilation() == 1.0f)
	{
		World->GetWorldSettings()->SetTimeDilation(0.2f);
		DebugUserWidget->SetSlowMotionTextActive(true);
	}
	else
	{
		World->GetWorldSettings()->SetTimeDilation(1.0f);
		DebugUserWidget->SetSlowMotionTextActive(false);
	}
}

void UDebugComponent::UpdateCharacterStateText() const
{
	if(!IsValid(DebugUserWidget))
	{
		return;
	}

	DebugUserWidget->SetAllowEnduranceRecoveryText(OwnerCharacter->GetAllowEnduranceRecovery());
	
	DebugUserWidget->SetAllowMovementText(OwnerCharacter->GetAllowMovement());

	bool bForcedMovement;
	float ForcedMovementRate;
	FVector ForcedMovementDirection;
	OwnerCharacter->GetForcedMovement(bForcedMovement, ForcedMovementRate, ForcedMovementDirection);
	DebugUserWidget->SetForcedMovementText(bForcedMovement);

	DebugUserWidget->SetForcedMovementRateText(ForcedMovementRate);
	DebugUserWidget->SetForcedMovementDirectionText(ForcedMovementDirection);

	DebugUserWidget->SetIsAttackingText(OwnerCharacter->GetIsAttacking());

	DebugUserWidget->SetIsBlockingText(OwnerCharacter->GetIsBlocking());

	DebugUserWidget->SetIsInvincibleText(OwnerCharacter->GetIsInvincible());
	
	DebugUserWidget->SetIsKickingText(OwnerCharacter->GetIsKicking());

	DebugUserWidget->SetIsLockedOnText(OwnerCharacter->GetIsLockedOn());
	
	DebugUserWidget->SetIsParryingText(OwnerCharacter->GetIsParrying());

	DebugUserWidget->SetIsRollingText(OwnerCharacter->GetIsRolling());
	
	DebugUserWidget->SetIsRunningText(OwnerCharacter->GetIsRunning());

	DebugUserWidget->SetIsStaggeredText(OwnerCharacter->GetIsStaggered());
	
	DebugUserWidget->SetIsStunnedText(OwnerCharacter->GetIsStunned());
	
	DebugUserWidget->SetParryCanStaggerText(OwnerCharacter->GetParryCanStagger());
}



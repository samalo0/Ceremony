// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugUserWidget.generated.h"

class UTextBlock;

/**
 * Base class for the debug widget.
 */
UCLASS()
class CEREMONY_API UDebugUserWidget : public UUserWidget
{

	GENERATED_BODY()

public:

	static void SetActiveColor(UTextBlock* TextBlock, bool bIsActive);

	void SetAllowEnduranceRecoveryText(bool bIsActive) const;
	
	void SetAllowMovementText(bool bAllowMovement) const;

	void SetAttackCanDamageText(const bool bCanDamage) const { SetActiveColor(AttackCanDamageText, bCanDamage); }
	
	void SetForcedMovementDirectionText(FVector ForcedMovementDirection) const;
	
	void SetForcedMovementRateText(float ForcedMovementRate) const;
		
	void SetForcedMovementText(bool bForcedMovement) const;

	void SetIsAttackingText(const bool bIsActive) const { SetActiveColor(IsAttackingText, bIsActive); }

	void SetIsBlockingText(const bool bIsActive) const { SetActiveColor(IsBlockingText, bIsActive); }

	void SetIsInvincibleText(const bool bIsActive) const { SetActiveColor(IsInvincibleText, bIsActive); }

	void SetIsKickingText(const bool bIsActive) const { SetActiveColor(IsKickingText, bIsActive); }

	void SetIsLockedOnText(const bool bIsActive) const { SetActiveColor(IsLockedOnText, bIsActive); }
	
	void SetIsParryingText(const bool bIsActive) const { SetActiveColor(IsParryingText, bIsActive); }

	void SetIsRollingText(const bool bIsActive) const { SetActiveColor(IsRollingText, bIsActive); }
	
	void SetIsRunningText(bool bIsActive) const;

	void SetIsStaggeredText(const bool bIsActive) const { SetActiveColor(IsStaggeredText, bIsActive); }
	
	void SetIsStunnedText(const bool bIsActive) const { SetActiveColor(IsStunnedText, bIsActive); }
	
	void SetKickCanDamageText(const bool bIsActive) const { SetActiveColor(KickCanDamageText, bIsActive); }
	
	void SetNetModeText(ENetMode NetMode) const;

	void SetNetRoleText(ENetRole NetRole) const;

	void SetParryCanStaggerText(const bool bIsActive) const { SetActiveColor(ParryCanStaggerText, bIsActive); }
	
	void SetShowCollisionTextActive(bool bIsActive) const;
	
	void SetShowIKTraceTextActive(bool bIsActive) const;
	
	void SetSlowMotionTextActive(bool bIsActive) const;
	
protected:

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AllowEnduranceRecoveryText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* AllowMovementText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AttackCanDamageText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ForcedMovementText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ForcedMovementDirectionText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ForcedMovementRateText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* KickCanDamageText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsAttackingText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsBlockingText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsInvincibleText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsKickingText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsLockedOnText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsParryingText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsRollingText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsRunningText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsStaggeredText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* IsStunnedText;
		
	UPROPERTY(meta=(BindWidget))
	UTextBlock* NetModeText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* NetRoleText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ParryCanStaggerText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShowCollisionText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShowIKTraceText;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* SlowMotionText;
	
};

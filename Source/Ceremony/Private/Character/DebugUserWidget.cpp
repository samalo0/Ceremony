// Copyright 2020 Stephen Maloney

#include "Character/DebugUserWidget.h"

#include "UMG/Public/Components/TextBlock.h"

void UDebugUserWidget::SetActiveColor(UTextBlock* TextBlock, const bool bIsActive)
{
	if(bIsActive)
	{
		TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	}
	else
	{
		TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
}

void UDebugUserWidget::SetAllowEnduranceRecoveryText(const bool bIsActive) const
{
	SetActiveColor(AllowEnduranceRecoveryText, !bIsActive);
}

void UDebugUserWidget::SetAllowMovementText(const bool bAllowMovement) const
{
	SetActiveColor(AllowMovementText, !bAllowMovement);
}

void UDebugUserWidget::SetForcedMovementDirectionText(const FVector ForcedMovementDirection) const
{
	ForcedMovementDirectionText->SetText(FText::FromString(FString::Printf(TEXT("Dir: %.2f X,%.2f Y"), ForcedMovementDirection.X, ForcedMovementDirection.Y)));

	if(ForcedMovementDirection.IsNearlyZero())
	{
		SetActiveColor(ForcedMovementDirectionText, false);
	}
	else
	{
		SetActiveColor(ForcedMovementDirectionText, true);
	}
}

void UDebugUserWidget::SetForcedMovementRateText(const float ForcedMovementRate) const
{
	ForcedMovementRateText->SetText(FText::FromString(FString::Printf(TEXT("Rate: %.3f"), ForcedMovementRate)));
	
	if(FMath::IsNearlyZero(ForcedMovementRate))
	{
		SetActiveColor(ForcedMovementRateText, false);
	}
	else
	{
		SetActiveColor(ForcedMovementRateText, true);
	}
}

void UDebugUserWidget::SetForcedMovementText(const bool bForcedMovement) const
{
	SetActiveColor(ForcedMovementText, bForcedMovement);
}

void UDebugUserWidget::SetIsRunningText(const bool bIsActive) const
{
	SetActiveColor(IsRunningText, bIsActive);
}

void UDebugUserWidget::SetNetModeText(const ENetMode NetMode) const
{
	switch(NetMode)
	{
	case NM_Client:
		NetModeText->SetText(FText::FromString(TEXT("NetMode: Client")));
		break;
	case NM_DedicatedServer:
		NetModeText->SetText(FText::FromString(TEXT("NetMode: DedicatedServer")));
		break;
	case NM_ListenServer:
		NetModeText->SetText(FText::FromString(TEXT("NetMode: ListenServer")));
		break;
	case NM_Standalone:
		NetModeText->SetText(FText::FromString(TEXT("NetMode: Standalone")));
		break;
	default:
		NetModeText->SetText(FText::FromString(TEXT("NetMode: Unknown")));
		break;
	}
}

void UDebugUserWidget::SetNetRoleText(const ENetRole NetRole) const
{
	switch(NetRole)
	{
	case ROLE_Authority:
		NetRoleText->SetText(FText::FromString(TEXT("NetRole: Authority")));
		break;
	case ROLE_AutonomousProxy:
		NetRoleText->SetText(FText::FromString(TEXT("NetRole: AutonomousProxy")));
		break;
	case ROLE_SimulatedProxy:
		NetRoleText->SetText(FText::FromString(TEXT("NetRole: SimulatedProxy")));
		break;
	case ROLE_None:
		NetRoleText->SetText(FText::FromString(TEXT("NetRole: None")));
		break;
	default:
		NetRoleText->SetText(FText::FromString(TEXT("NetRole: Unknown")));
		break;
	}
}

void UDebugUserWidget::SetShowCollisionTextActive(const bool bIsActive) const
{
	SetActiveColor(ShowCollisionText, bIsActive);
}

void UDebugUserWidget::SetShowIKTraceTextActive(const bool bIsActive) const
{
	SetActiveColor(ShowIKTraceText, bIsActive);
}

void UDebugUserWidget::SetSlowMotionTextActive(const bool bIsActive) const
{
	SetActiveColor(SlowMotionText, bIsActive);
}

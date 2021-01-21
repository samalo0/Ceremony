// Copyright 2020 Stephen Maloney

#include "Character/CeremonyOpponentUserWidget.h"

#include "Components/Image.h"
#include "Slate/SObjectWidget.h"
#include "UMG/Public/Components/TextBlock.h"

void UCeremonyOpponentUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	HealthBarDynamicMaterial = HealthBar->GetDynamicMaterial();

	HealthBarDynamicMaterial->SetVectorParameterValue("LosingColor", FLinearColor(0.5f, 0.5f, 0.0f));
	
	TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
	if(SafeGCWidget.IsValid())
	{
		SafeGCWidget->SetCanTick(false);
	}

	DamageText->SetVisibility(ESlateVisibility::Hidden);
	SetVisibility(ESlateVisibility::Hidden);
}

void UCeremonyOpponentUserWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(FMath::IsNearlyEqual(HealthCurrentValue, HealthTargetValue))
	{
		// Set timer to hide bar.
		if(!bHealthBarStayVisible)
		{
			UWorld* World = GetWorld();
			if(IsValid(World))
			{
				World->GetTimerManager().SetTimer(HealthBarHideTimerHandle, this, &UCeremonyOpponentUserWidget::OnHide, HideDelay);
			}
		}
		
		TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
		if(SafeGCWidget.IsValid())
		{
			SafeGCWidget->SetCanTick(false);
		}	
	}
	else
	{
		HealthCurrentValue = FMath::FInterpConstantTo(HealthCurrentValue, HealthTargetValue, InDeltaTime, InterpolationSpeed);
		HealthBarDynamicMaterial->SetScalarParameterValue("CurrentValue", HealthCurrentValue);
	}
}

void UCeremonyOpponentUserWidget::OnDamageChanged(const float Damage)
{
	DamageText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Damage)));
	DamageText->SetVisibility(ESlateVisibility::Visible);

	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		World->GetTimerManager().SetTimer(DamageTimerHandle, this, &UCeremonyOpponentUserWidget::OnHideDamage, HideDelay);
	}
}

void UCeremonyOpponentUserWidget::OnHealthChanged(const float Health, const float HealthMax)
{
	// Cancel hide timer.
	UWorld* World = GetWorld();
	if(IsValid(World) && HealthBarHideTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(HealthBarHideTimerHandle);
		HealthBarHideTimerHandle.Invalidate();
	}

	// Show the widget and update the target health.
	SetVisibility(ESlateVisibility::Visible);

	HealthTargetValue = Health;

	HealthBarDynamicMaterial->SetScalarParameterValue("MaximumValue", HealthMax);
	HealthBarDynamicMaterial->SetScalarParameterValue("TargetValue", HealthTargetValue);

	// Requires tick to interpolate.
	TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
	if(SafeGCWidget.IsValid())
	{
		SafeGCWidget->SetCanTick(true);
	}
}

void UCeremonyOpponentUserWidget::OnHide()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UCeremonyOpponentUserWidget::OnHideDamage() const
{
	DamageText->SetVisibility(ESlateVisibility::Hidden);
}

void UCeremonyOpponentUserWidget::SetHealthBarStayVisible(const bool bStayVisible)
{
	bHealthBarStayVisible = bStayVisible;

	if(bHealthBarStayVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// Set timer to hide.
		UWorld* World = GetWorld();
		if(IsValid(World))
		{
			World->GetTimerManager().SetTimer(HealthBarHideTimerHandle, this, &UCeremonyOpponentUserWidget::OnHide, HideDelay);
		}
	}
}

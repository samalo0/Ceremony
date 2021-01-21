// Copyright 2020 Stephen Maloney

#include "Character/CeremonyUserWidget.h"

#include "Components/Image.h"
#include "Slate/SObjectWidget.h"

float UCeremonyUserWidget::InterpolateBar(const float Current, const float Target, const float DeltaTime, const float DownInterpolationSpeed) const
{
	const float Dist = Target - Current;

	// If distance is too small, or growing larger, just set desired.
	if(FMath::Square(Dist) < SMALL_NUMBER || Dist > 0)
	{
		return Target;
	}

	// Falling values are interpolated.
	const float Step = -1.0f * DownInterpolationSpeed * DeltaTime;
	return Current + FMath::Clamp<float>(Dist, Step, 0);
}

void UCeremonyUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnduranceDynamicMaterial = EnduranceBar->GetDynamicMaterial();
	EnduranceDynamicMaterial->SetVectorParameterValue("FullColor", FLinearColor(0.0f, 1.0f, 0.0f));

	HealthDynamicMaterial = HealthBar->GetDynamicMaterial();

	TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
	if(SafeGCWidget.IsValid())
	{
		SafeGCWidget->SetCanTick(false);
	}
}

void UCeremonyUserWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	EnduranceCurrentValue = InterpolateBar(EnduranceCurrentValue, EnduranceTargetValue, InDeltaTime, InterpolationSpeed);
	EnduranceDynamicMaterial->SetScalarParameterValue("CurrentValue", EnduranceCurrentValue);
	
	HealthCurrentValue = InterpolateBar(HealthCurrentValue, HealthTargetValue, InDeltaTime, InterpolationSpeed);
	HealthDynamicMaterial->SetScalarParameterValue("CurrentValue", HealthCurrentValue);
	
	if(FMath::IsNearlyEqual(HealthCurrentValue, HealthTargetValue) && FMath::IsNearlyEqual(EnduranceCurrentValue, EnduranceTargetValue))
	{
		TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
		if(SafeGCWidget.IsValid())
		{
			SafeGCWidget->SetCanTick(false);
		}
	}
}

void UCeremonyUserWidget::OnEnduranceChanged(const float Current, const float Max)
{
	EnduranceTargetValue = Current;

	EnduranceDynamicMaterial->SetScalarParameterValue("MaximumValue", Max);
	EnduranceDynamicMaterial->SetScalarParameterValue("TargetValue", Current);

	if(!FMath::IsNearlyEqual(EnduranceCurrentValue, EnduranceTargetValue))
	{
		TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
		if(SafeGCWidget.IsValid())
		{
			SafeGCWidget->SetCanTick(true);
		}
	}
}

void UCeremonyUserWidget::OnHealthChanged(const float Current, const float Max)
{
	HealthTargetValue = Current;

	HealthDynamicMaterial->SetScalarParameterValue("MaximumValue", Max);
	HealthDynamicMaterial->SetScalarParameterValue("TargetValue", Current);
	
	if(!FMath::IsNearlyEqual(HealthCurrentValue, HealthTargetValue))
	{
		TSharedPtr<SObjectWidget> SafeGCWidget = MyGCWidget.Pin();
		if(SafeGCWidget.IsValid())
		{
			SafeGCWidget->SetCanTick(true);
		}
	}
}


// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CeremonyUserWidget.generated.h"

class UImage;

constexpr float InterpolationSpeed = 20.0f;

/**
 * Class for the character health and endurance user widget.
 */
UCLASS()
class CEREMONY_API UCeremonyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Function to call to update the endurance display.
	UFUNCTION()
	void OnEnduranceChanged(float Current, float Max);

	// Function to call to update health display.
	UFUNCTION()
	void OnHealthChanged(float Current, float Max);

	void NativeConstruct() override;
	
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:

	float InterpolateBar(float Current, float Target, float DeltaTime, float DownInterpolationSpeed) const;
	
	// Automatically bind this reference to the endurance bar widget inside the child blueprint.
	UPROPERTY(meta=(BindWidget))
	UImage* EnduranceBar;

	float EnduranceCurrentValue = 100.0f;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* EnduranceDynamicMaterial;
	
	float EnduranceTargetValue = 100.0f;
	
	UPROPERTY(meta=(BindWidget))
	UImage* HealthBar;

	// The current value of health displayed on the bar.
	float HealthCurrentValue = 100.0f;
	
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* HealthDynamicMaterial;
	
	// The actual value of health, which will be interpolated.
	float HealthTargetValue = 100.0f;
	
};

// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CeremonyOpponentUserWidget.generated.h"

/**
 * Class for the opponent health user widget.
 */
UCLASS()
class CEREMONY_API UCeremonyOpponentUserWidget : public UUserWidget
{

	GENERATED_BODY()

public:

	void NativeOnInitialized() override;
	
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Function to call to update the damage text.
	UFUNCTION()
	void OnDamageChanged(float Damage);
	
	// Function to call to update health display.
	UFUNCTION()
	void OnHealthChanged(float Health, float HealthMax);

	// Causes the health bar to stay visible always and not hide.
	void SetHealthBarStayVisible(bool bStayVisible);
	
protected:

	void OnHide();

	void OnHideDamage() const;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* DamageText;

	FTimerHandle DamageTimerHandle;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* HealthBar;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* HealthBarDynamicMaterial;

	float HealthTargetValue = 100.0f;

	float HealthCurrentValue = 100.0f;

	// Amount of time before hiding the health bar when not always visible.
	UPROPERTY(EditDefaultsOnly)
	float HideDelay = 3.0f;
	
	FTimerHandle HealthBarHideTimerHandle;

	const float InterpolationSpeed = 20.0f;

	bool bHealthBarStayVisible = false;
	
};

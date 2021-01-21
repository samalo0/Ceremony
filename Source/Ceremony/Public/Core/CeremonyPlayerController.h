// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CeremonyPlayerController.generated.h"

class UCeremonyUserWidget;
class UDebugUserWidget;

/**
 * Default player controller for Ceremony. Allows creating/placing the HUD.
 */
UCLASS()
class CEREMONY_API ACeremonyPlayerController : public APlayerController
{

	GENERATED_BODY()

public:

	// The pointer to the widget once created.
	UPROPERTY(Transient)
	UCeremonyUserWidget* CharacterWidget;

	// The pointer to the widget once created.
	UPROPERTY(Transient)
	UDebugUserWidget* DebugWidget;
	
protected:

	void BeginPlay() override;

	// The child class to spawn for the character widget.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCeremonyUserWidget> CharacterWidgetClass;

	// The child class to spawn for the debug widget.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDebugUserWidget> DebugWidgetClass;
};

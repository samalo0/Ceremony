// Copyright 2020 Stephen Maloney

#include "Core/CeremonyPlayerController.h"

#include "Character/CeremonyUserWidget.h"
#include "Character/DebugUserWidget.h"

void ACeremonyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only create widgets on the local player; don't create widgets for all other players (that you can't see anyway)
	if(!IsLocalPlayerController())
	{
		return;
	}

	if(IsValid(CharacterWidgetClass))
	{
		CharacterWidget = CreateWidget<UCeremonyUserWidget>(this, CharacterWidgetClass, TEXT("CeremonyWidget"));
		if(IsValid(CharacterWidget))
		{
			CharacterWidget->AddToViewport();	
		}
	}
	
	if(IsValid(DebugWidgetClass))
	{
		DebugWidget = CreateWidget<UDebugUserWidget>(this, DebugWidgetClass, TEXT("DebugWidget"));
		if(IsValid(DebugWidget))
		{
			DebugWidget->AddToViewport();	
		}
	}
}

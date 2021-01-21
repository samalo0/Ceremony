// Copyright 2020 Stephen Maloney

#include "Core/CeremonyGameInstance.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UCeremonyGameInstance::Host() const
{
	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to get world."));
		return;
	}

	World->ServerTravel("/Game/Ceremony/Maps/RoundArena_P?listen");
}

void UCeremonyGameInstance::Join(const FString& Address) const
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if(!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to get world."));
		return;
	}

	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}


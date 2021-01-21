// Copyright 2020 Stephen Maloney

#include "Core/CeremonyGameModeBase.h"

#include "Character/CeremonyCharacter.h"
#include "Engine/World.h"

ACeremonyGameModeBase::ACeremonyGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ACeremonyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	ACeremonyCharacter* Character = Cast<ACeremonyCharacter>(NewPlayer->GetCharacter());
	if(IsValid(Character))
	{
		// Set the player number, which is replicated so that clients will set colors appropriately.
		Character->ClientPlayerNumber = GetNumPlayers() % 4;

		// On the server, call OnRep directly to force the server to set colors.
		if(GetNetMode() == NM_ListenServer)
		{
			Character->OnRep_ClientPlayerNumber();	
		}
	}
}

void ACeremonyGameModeBase::RestartDeadPlayers()
{
	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		for(FConstPlayerControllerIterator WeakPlayerController = World->GetPlayerControllerIterator(); WeakPlayerController; ++WeakPlayerController)
		{
			APlayerController* Controller = WeakPlayerController->Get();
			if(IsValid(Controller))
			{
				ACeremonyCharacter* Character = Cast<ACeremonyCharacter>(Controller->GetCharacter());
				if(!IsValid(Character))
				{
					RestartPlayer(Controller);
				}
			}
		}
	}
}

void ACeremonyGameModeBase::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RestartDeadPlayers();
}


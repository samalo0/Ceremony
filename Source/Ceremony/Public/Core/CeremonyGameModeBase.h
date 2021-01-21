// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CeremonyGameModeBase.generated.h"

/**
 * Base game mode for Ceremony.
 */
UCLASS()
class CEREMONY_API ACeremonyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACeremonyGameModeBase();

	void PostLogin(APlayerController* NewPlayer) override;

	void Tick(float DeltaSeconds) override;

protected:

	void RestartDeadPlayers();
};

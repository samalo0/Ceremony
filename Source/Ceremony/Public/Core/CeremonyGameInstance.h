// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CeremonyGameInstance.generated.h"

/**
 * Base game instance.
 */
UCLASS()
class CEREMONY_API UCeremonyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(Exec)
	void Host() const;

	UFUNCTION(Exec)
	void Join(const FString& Address) const;
	
};

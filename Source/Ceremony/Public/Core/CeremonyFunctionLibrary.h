// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CeremonyFunctionLibrary.generated.h"

/**
 * Blueprint library for commonly used functions.
 */
UCLASS()
class CEREMONY_API UCeremonyFunctionLibrary : public UBlueprintFunctionLibrary
{

	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static void LogRoleAndMode(APawn* Pawn, FString InfoString);
	
};

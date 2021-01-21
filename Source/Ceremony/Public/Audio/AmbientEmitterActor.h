// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmbientEmitterActor.generated.h"

/**
 * Base class for ambient audio emitters. It plays the audio specified in the
 * audio component repeatedly between minimum and maximum delay at a random
 * location in the bounding box.
 */
UCLASS()
class CEREMONY_API AAmbientEmitterActor : public AActor
{

	GENERATED_BODY()
	
public:	

	AAmbientEmitterActor();
	
protected:
	
	void BeginPlay() override;

	void PlaySoundAtRandomPointInBox() const;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=2.0f, ClampMax=60.0f))
	float MaximumDelay = 35.0f;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=1.0f, ClampMax=10.0f))
	float MinimumDelay = 7.0f;
	
#pragma region Components

	UPROPERTY(VisibleAnywhere)
	class UAudioComponent* AudioComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent;

#pragma endregion

};

// Copyright 2020 Stephen Maloney

#include "Audio/AmbientEmitterActor.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

AAmbientEmitterActor::AAmbientEmitterActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionProfileName(TEXT("NoCollision"));
	BoxComponent->SetMobility(EComponentMobility::Static);
	BoxComponent->SetGenerateOverlapEvents(false);
	SetRootComponent(BoxComponent);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
}

void AAmbientEmitterActor::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, this, &AAmbientEmitterActor::PlaySoundAtRandomPointInBox, FMath::RandRange(MinimumDelay, MaximumDelay));
	}
}

void AAmbientEmitterActor::PlaySoundAtRandomPointInBox() const
{
	const FVector BoxMin = BoxComponent->GetComponentLocation() - BoxComponent->GetScaledBoxExtent();
	const FVector BoxMax = BoxComponent->GetComponentLocation() + BoxComponent->GetScaledBoxExtent();
	const FVector Location = FMath::RandPointInBox(FBox(BoxMin, BoxMax));

	AudioComponent->SetWorldLocation(Location);

	AudioComponent->Play();

	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, this, &AAmbientEmitterActor::PlaySoundAtRandomPointInBox, FMath::RandRange(MinimumDelay, MaximumDelay));
	}
}


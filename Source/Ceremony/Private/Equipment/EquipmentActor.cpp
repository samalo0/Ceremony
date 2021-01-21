// Copyright 2020 Stephen Maloney

#include "Equipment/EquipmentActor.h"

#include "Character/CeremonyCharacter.h"
#include "Net/UnrealNetwork.h"

AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate equipment spawned on the server to clients.
	SetReplicates(true);
}

void AEquipmentActor::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACeremonyCharacter>(GetOwner());	
}

void AEquipmentActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate the equipment state to owning client.
	DOREPLIFETIME_CONDITION(AEquipmentActor, EquipmentState, COND_OwnerOnly);
}


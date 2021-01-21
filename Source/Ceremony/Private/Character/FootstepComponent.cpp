// Copyright 2020 Stephen Maloney

#include "Character/FootstepComponent.h"

#include "Character/CeremonyCharacter.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"

UFootstepComponent::UFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACeremonyCharacter>(GetOwner());
	check(IsValid(OwnerCharacter));

	OwnerMesh = OwnerCharacter->GetMesh();
	check(IsValid(OwnerMesh));

	check(IsValid(FootstepDataTable));
	
	CollisionQueryParams.AddIgnoredActor(OwnerCharacter);
	CollisionQueryParams.bReturnPhysicalMaterial = true;
}

USoundBase* UFootstepComponent::GetFootstepSound(const FName FootBoneName) const
{
	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		return nullptr;
	}

	FHitResult OutHit;
	const FVector StartLocation = OwnerMesh->GetBoneLocation(FootBoneName);
	const FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, FootTraceLength);

	if(OwnerCharacter->IsShowingDebugCollision())
	{
		DrawDebugLine(World, StartLocation, EndLocation, FColor::Red, false, 0.5f, 0, 0);
	}

	if(World->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_Visibility, CollisionQueryParams))
	{
		UPhysicalMaterial* PhysicalMaterial = OutHit.PhysMaterial.Get();
		const FName PhysicalMaterialName = FName(*PhysicalMaterial->GetName());
		const FFootStepSound* Sound = FootstepDataTable->FindRow<FFootStepSound>(PhysicalMaterialName, "", false);

		if(bShowDebug)
		{
			if(Sound != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("FootstepComponent: Line trace hit actor %s, physical material %s, sound is %s."), *GetNameSafe(OutHit.GetActor()), *PhysicalMaterialName.ToString(), *GetNameSafe(Sound->FootstepSound));	
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FootstepComponent: Line trace hit actor %s, physical material %s, no sound found."), *GetNameSafe(OutHit.GetActor()), *PhysicalMaterialName.ToString());	
			}
		}
		
		if(Sound != nullptr)
		{
			OwnerCharacter->PlaySound(Sound->FootstepSound);
		}
	}

	return nullptr;
}

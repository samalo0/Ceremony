// Copyright 2020 Stephen Maloney

#include "Equipment/MeleeWeaponActor.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/CeremonyCharacter.h"
#include "Character/CeremonyMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AMeleeWeaponActor::AMeleeWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("SceneComponent"));
	ArrowComponent->ArrowSize = 0.25f;
	SetRootComponent(ArrowComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCollisionProfileName("OverlapOnlyPawn");
	CapsuleComponent->SetGenerateOverlapEvents(false);
	CapsuleComponent->SetCapsuleRadius(3.0f);
	CapsuleComponent->SetCapsuleHalfHeight(48.0f);
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeaponActor::OnCapsuleComponentOverlap);
	CapsuleComponent->SetupAttachment(StaticMeshComponent);
}

void AMeleeWeaponActor::CancelActions()
{
	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMeleeWeaponActor::CancelActions: Cancelling actions for character %s on %s."), *GetNameSafe(OwnerCharacter), *GetNameSafe(this));
	}

	bPress1OnResume = false;
	
	OwnerCharacter->ClearOnMontageEndedDelegate();
	OwnerCharacter->StopMontageGlobally();

	if(Press2FullyChargedTimerHandle.IsValid())
	{
		const UWorld* World = GetWorld();
		if(IsValid(World))
		{
			World->GetTimerManager().ClearTimer(Press2FullyChargedTimerHandle);	
		}
		Press2FullyChargedTimerHandle.Invalidate();
	}
	
	CapsuleComponent->SetGenerateOverlapEvents(false);
	
	bPress1IsAttacking = false;
	bPress1QueueNextAttack = false;
	Press1CurrentAttack = 0;

	bPress2IsAttacking = false;
	bPress2QueueNextAttack = false;
		
	OwnerCharacter->SetAllowEnduranceRecovery(true);
	OwnerCharacter->SetIsAttacking(false);
}

void AMeleeWeaponActor::ShowCollision(const bool bShow)
{
	CapsuleComponent->SetHiddenInGame(!bShow);
}

#pragma region Attack

void AMeleeWeaponActor::CheckForAttackTransition()
{
	// If the next attack is queued, transition to it.
	if((!bPress1QueueNextAttack && !bPress2QueueNextAttack) || OwnerCharacter->GetEndurance() <= 0.0f)
	{
		return;
	}

	if(bPress1QueueNextAttack)
	{
		// Must press attack to queue the next attack again.
		bPress1QueueNextAttack = false;

		// Increment which attack is being played.
		Press1CurrentAttack++;
		if(Press1CurrentAttack >= Press1AttackParams.Num())
		{
			Press1CurrentAttack = 0;
		}

		// Set up damage in case a hit occurs.
		DamageEnduranceDamageStunTime = FVector(Press1AttackParams[Press1CurrentAttack].DamageParams.DamageStandard,
			Press1AttackParams[Press1CurrentAttack].DamageParams.EnduranceDamageStandard,
				Press1AttackParams[Press1CurrentAttack].DamageParams.StunTime);
		DamageType = static_cast<uint8>(Press1AttackParams[Press1CurrentAttack].DamageParams.DamageType);
		
		// Play the next attack and jump to the active section.
		OwnerCharacter->DepleteEndurance(Press1AttackParams[Press1CurrentAttack].EnduranceConsumption);
		OwnerCharacter->ClearOnMontageEndedDelegate();
		OwnerCharacter->PlayMontageGlobally(Press1AttackParams[Press1CurrentAttack].Montage, Press1AttackParams[Press1CurrentAttack].MontageActiveJumpSectionName);
		OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press1AttackParams[Press1CurrentAttack].Montage);
	}
	else
	{
		OwnerCharacter->ClearOnMontageEndedDelegate();
		bPress1IsAttacking = false;
		bPress2QueueNextAttack = false;
		TriggerPress2Attack();
	}
}

void AMeleeWeaponActor::OnAttackMontageEnded(UAnimMontage* Montage, const bool bInterrupted)
{
	OwnerCharacter->ClearOnMontageEndedDelegate();

	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMeleeWeaponActor::OnAttackMontageEnded: Character %s objects %s Montage %s Interrupted %d"), *GetNameSafe(OwnerCharacter), *GetNameSafe(this), *GetNameSafe(Montage), bInterrupted);
	}
	
	if(bPress1QueueNextAttack || bPress2QueueNextAttack && OwnerCharacter->GetEndurance() > 0.0f)
	{
		if(bPress1QueueNextAttack)
		{
			bPress2IsAttacking = false;
			bPress2QueueNextAttack = false;

			// Must press attack to queue the next attack again.
			bPress1QueueNextAttack = false;

			// Increment which attack is being played.
			Press1CurrentAttack++;
			if(Press1CurrentAttack >= Press1AttackParams.Num())
			{
				Press1CurrentAttack = 0;
			}

			// Play the next attack.
			OwnerCharacter->DepleteEndurance(Press1AttackParams[Press1CurrentAttack].EnduranceConsumption);
			OwnerCharacter->PlayMontageGlobally(Press1AttackParams[Press1CurrentAttack].Montage);
			OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press1AttackParams[Press1CurrentAttack].Montage);
		}
		else
		{
			// Press2 occurred while playing a press1 attack.
			bPress1IsAttacking = false;
			bPress2IsAttacking = false;
			bPress2QueueNextAttack = false;
			TriggerPress2Attack();
		}
	}
	else
	{
		bPress1IsAttacking = false;
		bPress1QueueNextAttack = false;
		Press1CurrentAttack = 0;

		bPress2IsAttacking = false;
		bPress2QueueNextAttack = false;
		
		OwnerCharacter->StopMontageGlobally();
		OwnerCharacter->SetAllowMovement(true);
		OwnerCharacter->SetAllowEnduranceRecovery(true);
		OwnerCharacter->SetIsAttacking(false);
		OwnerCharacter->CheckForResumingAction();
	}
}

void AMeleeWeaponActor::OnCapsuleComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Prevent hitting a character more than once with the same attack.
	if(DamagedActors.Contains(OtherActor))
	{
		return;
	}

	const UWorld* World = GetWorld();
	if(!IsValid(World) || !IsValid(OwnerCharacter))
	{
		return;
	}

	// They have overlapped, we need to get the location of the overlap by sweeping a small distance.
	TArray<FHitResult> OutHits;

	const FVector Start = CapsuleComponent->GetComponentLocation();
	const FVector End = FVector(Start.X + 1.0f, Start.Y + 1.0f, Start.Z + 1.0f);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.AddIgnoredActors(DamagedActors);
	
	if(World->SweepMultiByObjectType(OutHits, Start, End, CapsuleComponent->GetComponentRotation().Quaternion(), ObjectQueryParams, CapsuleComponent->GetCollisionShape(), Params))
	{
		for(auto OutHit : OutHits)
		{
			if(OutHit.Actor == OtherActor && OutHit.GetComponent() == OtherComp)
			{
				if(OwnerCharacter->IsShowingDebugCollision())
				{
					DrawDebugCapsule(World, Start, CapsuleComponent->GetScaledCapsuleHalfHeight(),
					                 CapsuleComponent->GetScaledCapsuleRadius(),
					                 CapsuleComponent->GetComponentRotation().Quaternion(), FColor::Red, false, 3.0f, 0, 0);
					
					DrawDebugSphere(World, OutHit.ImpactPoint, 3.0f, 8, FColor::Red, false, 3.0f, 0, 1);
				}

				DamagedActors.Add(OtherActor);

				ACeremonyCharacter* CharacterHit = Cast<ACeremonyCharacter>(OutHit.GetActor());
				
				OwnerCharacter->Server_VerifyOverlapForDamage(CharacterHit, OutHit.ImpactPoint, DamageEnduranceDamageStunTime, DamageType);
				break;
			}
		}
	}
}

void AMeleeWeaponActor::SetAttackCanDamage(const bool bCanDamage)
{
	if(bCanDamage)
	{
		DamagedActors.Empty();
		CapsuleComponent->SetGenerateOverlapEvents(true);
	}
	else
	{
		CapsuleComponent->SetGenerateOverlapEvents(false);
	}
}

#pragma endregion

#pragma region Press1

void AMeleeWeaponActor::Press1()
{
	if(OwnerCharacter->GetCanAttack())
	{
		if(OwnerCharacter->GetIsBlocking())
		{
			OwnerCharacter->CancelBlocking();
		}
		
		if(!bPress1IsAttacking && !bPress2IsAttacking)
		{
			// Start a new attack.
			bPress1IsAttacking = true;
			OwnerCharacter->SetAllowMovement(false);
			OwnerCharacter->SetAllowEnduranceRecovery(false);
			
			if(OwnerCharacter->GetIsRunning())
			{
				// Set up damage in case a hit occurs.
				DamageEnduranceDamageStunTime = FVector_NetQuantize(Press1RunningAttackParams.DamageParams.DamageStandard,
					Press1RunningAttackParams.DamageParams.EnduranceDamageStandard,
						Press1RunningAttackParams.DamageParams.StunTime);
				DamageType = static_cast<uint8>(Press1RunningAttackParams.DamageParams.DamageType);
				
				OwnerCharacter->SetIsRunning(false);
				OwnerCharacter->DepleteEndurance(Press1RunningAttackParams.EnduranceConsumption);
				OwnerCharacter->PlayMontageGlobally(Press1RunningAttackParams.Montage);
				OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press1RunningAttackParams.Montage);
			}
			else
			{
				// Check for special attacks
				ESpecialAttackType OutAttackType = ESpecialAttackType::None;
				ACeremonyCharacter* OutHitCharacter = CheckForSpecialAttack(OutAttackType);

				if(OutAttackType == ESpecialAttackType::BackStab)
				{
					OwnerCharacter->DepleteEndurance(BackStabEnduranceConsumption);
					OwnerCharacter->PlayMontageGlobally(BackStabMontage);
					OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", BackStabMontage);
					OwnerCharacter->Server_VerifyBackStab(OutHitCharacter, Press1AttackParams[0].DamageParams.DamageStandard * Press1AttackParams[0].DamageParams.BackStabMultiplier);
				}
				else if(OutAttackType == ESpecialAttackType::Riposte)
				{
					OwnerCharacter->DepleteEndurance(RiposteEnduranceConsumption);
					OwnerCharacter->PlayMontageGlobally(RiposteMontage);
					OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", RiposteMontage);
					OwnerCharacter->Server_VerifyRiposte(OutHitCharacter, Press1AttackParams[0].DamageParams.DamageStandard * Press1AttackParams[0].DamageParams.RiposteMultiplier);
				}
				else
				{
					// Set up damage in case a hit occurs.
					DamageEnduranceDamageStunTime = FVector_NetQuantize(Press1AttackParams[Press1CurrentAttack].DamageParams.DamageStandard,
						Press1AttackParams[Press1CurrentAttack].DamageParams.EnduranceDamageStandard,
							Press1AttackParams[Press1CurrentAttack].DamageParams.StunTime);
					DamageType = static_cast<uint8>(Press1AttackParams[Press1CurrentAttack].DamageParams.DamageType);

					OwnerCharacter->DepleteEndurance(Press1AttackParams[Press1CurrentAttack].EnduranceConsumption);
					OwnerCharacter->PlayMontageGlobally(Press1AttackParams[Press1CurrentAttack].Montage);
					OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press1AttackParams[Press1CurrentAttack].Montage);
				}
			}

			OwnerCharacter->SetIsAttacking(true);
		}
		else
		{
			// Already attacking, queue up the next attack.
			bPress1QueueNextAttack = true;
		}
	}
	else
	{
		bPress1OnResume = true;
	}
}

void AMeleeWeaponActor::Resume1(bool bHeldDown)
{
	if(bPress1OnResume)
	{
		bPress1OnResume = false;
		Press1();
	}
}

#pragma endregion 

#pragma region Press2

void AMeleeWeaponActor::Press2()
{
	bPress2IsHeldDown = true;
	
	TriggerPress2Attack();
}

void AMeleeWeaponActor::Release2()
{
	bPress2IsHeldDown = false;

	if(bPress2IsCharging)
	{
		const UWorld* World = GetWorld();
		if(!IsValid(World))
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to get world on %s."), *GetNameSafe(this));
			return;
		}

		if(Press2FullyChargedTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(Press2FullyChargedTimerHandle);
			Press2FullyChargedTimerHandle.Invalidate();
		}

		bPress2IsCharging = false;

		// Determine the amount of endurance consumption, damage, and endurance damage from the time charged.
		const float TimeDifference = FMath::Clamp(World->GetTimeSeconds() - Press2ChargeStartTimestamp, 0.0f, Press2AttackParams.ChargeSeconds);

		// Set up damage in case a hit occurs.
		DamageEnduranceDamageStunTime = FVector_NetQuantize(FMath::Lerp(Press2AttackParams.DamageParams.DamageStandard, Press2AttackParams.DamageParams.DamageFullyCharged, TimeDifference/Press2AttackParams.ChargeSeconds),
			FMath::Lerp(Press2AttackParams.DamageParams.EnduranceDamageStandard, Press2AttackParams.DamageParams.EnduranceDamageFullyCharged, TimeDifference/Press2AttackParams.ChargeSeconds),
				Press2AttackParams.DamageParams.StunTime);

		DamageType = static_cast<uint8>(Press2JumpingAttackParams.DamageParams.DamageType);
		
		const float EnduranceConsumed = FMath::Lerp(Press2AttackParams.EnduranceConsumptionMinimum, Press2AttackParams.EnduranceConsumptionMaximum, TimeDifference/Press2AttackParams.ChargeSeconds);

		OwnerCharacter->DepleteEndurance(EnduranceConsumed);

		OwnerCharacter->PlayMontageGlobally(Press2AttackParams.AttackMontage);
		OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press2AttackParams.AttackMontage);
	}
}

void AMeleeWeaponActor::Resume2(const bool bHeldDown)
{
	if(bHeldDown)
	{
		Press2();
	}
}

void AMeleeWeaponActor::TriggerPress2Attack()
{
	if(OwnerCharacter->GetCanAttack())
	{
		if(OwnerCharacter->GetIsBlocking())
		{
			OwnerCharacter->CancelBlocking();
		}
		
		if(!bPress2IsAttacking && !bPress1IsAttacking)
		{
			// Start a new attack.
			bPress2IsAttacking = true;

			OwnerCharacter->SetAllowEnduranceRecovery(false);
			OwnerCharacter->SetAllowMovement(false);
			
			if(OwnerCharacter->GetIsRunning())
			{
				// Set up damage in case a hit occurs.
				DamageEnduranceDamageStunTime = FVector_NetQuantize(Press2JumpingAttackParams.DamageParams.DamageStandard,
					Press2JumpingAttackParams.DamageParams.EnduranceDamageStandard,
						Press2JumpingAttackParams.DamageParams.StunTime);
				DamageType = static_cast<uint8>(Press2JumpingAttackParams.DamageParams.DamageType);

				OwnerCharacter->SetIsRunning(false);
				OwnerCharacter->DepleteEndurance(Press2JumpingAttackParams.EnduranceConsumption);

				OwnerCharacter->PlayMontageGlobally(Press2JumpingAttackParams.Montage);
				OwnerCharacter->SetOnMontageEndedDelegate(this, "OnAttackMontageEnded", Press2JumpingAttackParams.Montage);
			}
			else
			{
				const UWorld* World = GetWorld();
				if(!IsValid(World))
				{
					UE_LOG(LogTemp, Error, TEXT("Unable to get world on %s."), *GetNameSafe(this));
					return;
				}

				// Reset damage types, they will be set on release.
				DamageEnduranceDamageStunTime = FVector_NetQuantize::ZeroVector;
				DamageType = 0;
				
				OwnerCharacter->PlayMontageGlobally(Press2AttackParams.ChargeMontage);
				
				Press2ChargeStartTimestamp = World->GetTimeSeconds();

				bPress2IsCharging = true;

				if(bPress2IsHeldDown)
				{
					// Set up a timer to force the attack after being fully charged.
					World->GetTimerManager().SetTimer(Press2FullyChargedTimerHandle, this, &AMeleeWeaponActor::Release2, Press2AttackParams.ChargeSeconds);					
				}
				else
				{
					// Trigger instant release if the button isn't held down.
					Release2();
				}
			}

			OwnerCharacter->SetIsAttacking(true);
		}
		else
		{
			bPress2QueueNextAttack = true;
		}
	}
}

#pragma endregion

#pragma region SpecialAttack

ACeremonyCharacter* AMeleeWeaponActor::CheckForSpecialAttack(ESpecialAttackType& OutAttackType) const
{
	// Set up defaults
	ACeremonyCharacter* OutHitCharacter = nullptr;
	OutAttackType = ESpecialAttackType::None;

	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("AMeleeWeaponActor::CheckForSpecialAttack: Unable to get world for %s."), *GetNameSafe(this));
	}

	const FVector Start = OwnerCharacter->GetActorLocation();
	const FVector AttackingCharacterForwardVector = OwnerCharacter->GetActorForwardVector();
	const FVector End = Start + AttackingCharacterForwardVector * SpecialAttackReach;

	if(OwnerCharacter->IsShowingDebugCollision())
	{
		DrawDebugLine(World, Start, End, FColor::Red, false, 3.0f, 0, 0);
	}
	
	FHitResult OutHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	if(!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn, Params))
	{
		return nullptr;
	}

	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMeleeWeaponActor::CheckForSpecialAttack: Character %s hit actor %s, hit component %s."), *GetNameSafe(OwnerCharacter), *GetNameSafe(OutHit.GetActor()), *GetNameSafe(OutHit.GetComponent()));	
	}
	
	OutHitCharacter = Cast<ACeremonyCharacter>(OutHit.GetActor());
	if(!IsValid(OutHitCharacter))
	{
		return nullptr;
	}

	const FVector HitCharacterForwardVector = OutHitCharacter->GetActorForwardVector();

	if(OwnerCharacter->IsShowingDebugCollision())
	{
		DrawDebugLine(World, OutHitCharacter->GetActorLocation(), OutHitCharacter->GetActorLocation() + HitCharacterForwardVector * SpecialAttackReach, FColor::Green, false, 3.0f, 0, 0);
	}

	const float DotProduct = FVector::DotProduct(AttackingCharacterForwardVector, HitCharacterForwardVector);

	if(bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMeleeWeaponActor::CheckForSpecialAttack: Dot product is %f, riposte max %f, backstab min %f, hit character stagger %d."), DotProduct, RiposteDotProductMaximum, BackStabDotProductMinimum, OutHitCharacter->GetIsStaggered());	
	}
	
	if(OutHitCharacter->GetIsStaggered() && DotProduct <= RiposteDotProductMaximum)
	{
		OutAttackType = ESpecialAttackType::Riposte;

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("AMeleeWeaponActor::CheckForSpecialAttack: Character %s riposted character %s!"), *GetNameSafe(OwnerCharacter), *GetNameSafe(OutHitCharacter));	
		}
	}
	else if(DotProduct >= BackStabDotProductMinimum)
	{
		OutAttackType = ESpecialAttackType::BackStab;

		if(bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("AMeleeWeaponActor::CheckForSpecialAttack: Character %s backstabbed character %s!"), *GetNameSafe(OwnerCharacter), *GetNameSafe(OutHitCharacter));	
		}
	}

	return OutHitCharacter;
}

#pragma endregion
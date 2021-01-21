// Copyright 2020 Stephen Maloney

#include "Character/CeremonyCharacter.h"

#include "Animation/AnimInstance.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/CeremonyFunctionLibrary.h"
#include "Character/CeremonyMovementComponent.h"
#include "Character/CeremonyOpponentUserWidget.h"
#include "Core/CeremonyPlayerController.h"
#include "Character/CeremonyUserWidget.h"
#include "GameFramework/Controller.h"
#include "Character/DebugComponent.h"
#include "DrawDebugHelpers.h"
#include "Equipment/EquipmentActor.h"
#include "Character/FootstepComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "Character/InverseKinematicsComponent.h"
#include "Character/LockOnComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Equipment/ShieldActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"

ACeremonyCharacter::ACeremonyCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCeremonyMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Fully functioning character need to tick (update endurance).
	PrimaryActorTick.bCanEverTick = true;

	// Only fully functioning characters need to tick.
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Disable collision on the character mesh.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Disable the character from rotating due to controller rotation; this is used for the camera movement.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	// Rotate the character towards the direction that it's moved.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->AirControl = 0.0f;
	GetCharacterMovement()->JumpZVelocity = 300.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	
	// Create a spring arm to attack the third person camera.
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->TargetArmLength = 250.0f;
	SpringArmComponent->CameraLagSpeed = 5.0f;
	SpringArmComponent->CameraRotationLagSpeed = 5.0f;

	// Create a third person camera.
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	// Create the IK component.
	InverseKinematicsComponent = CreateDefaultSubobject<UInverseKinematicsComponent>(TEXT("InverseKinematicsComponent"));

	// Create a kick capsule collision component.
	KickCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("KickCapsuleComponent"));
	KickCapsuleComponent->SetupAttachment(GetMesh(), TEXT("ball_r"));
	KickCapsuleComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	KickCapsuleComponent->SetGenerateOverlapEvents(false);
	KickCapsuleComponent->SetCapsuleRadius(10.0f);
	KickCapsuleComponent->SetCapsuleHalfHeight(20.0f);
	
	// Create debug component for debug functionality.
	DebugComponent = CreateDefaultSubobject<UDebugComponent>(TEXT("DebugComponent"));

	// Create lock on component to allow locking on.
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
		
	// Footstep audio component.
	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComponent"));

	// Create a widget for displaying the character health and damage when hit by an opponent (display on the opponent's screen).
	OpponentWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OpponentWidget"));
	OpponentWidget->SetGenerateOverlapEvents(false);
	OpponentWidget->SetCollisionProfileName(TEXT("NoCollision"));
	OpponentWidget->SetWidgetSpace(EWidgetSpace::Screen);
	OpponentWidget->SetDrawSize(FVector2D(200.0f, 25.0f));
	OpponentWidget->SetupAttachment(GetRootComponent());

	// Create a widget for displaying lock on status.
	LockOnWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidget"));
	LockOnWidget->SetGenerateOverlapEvents(false);
	LockOnWidget->SetCollisionProfileName(TEXT("NoCollision"));
	LockOnWidget->SetWidgetSpace(EWidgetSpace::Screen);
	LockOnWidget->SetDrawSize(FVector2D(20.0f));
	LockOnWidget->SetVisibility(false);
	LockOnWidget->SetupAttachment(GetRootComponent());
}

void ACeremonyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCeremonyFunctionLibrary::LogRoleAndMode(this, FString::Printf(TEXT("BEGIN PLAY %s"), *GetNameSafe(this)));
	
	if(GetLocalRole() == ROLE_Authority)
	{
		Health = HealthMaximum;

		UWorld* World = GetWorld();
		check(IsValid(World));

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = this;
		const FTransform Transform = FTransform::Identity;
		
		// Spawn default equipment on the server.
		if(!IsValid(RightHandEquipmentDefaultClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot spawn null right hand equipment class for %s."), *GetNameSafe(this));
		}
		else
		{
			RightHandEquipment = World->SpawnActor<AEquipmentActor>(RightHandEquipmentDefaultClass, Transform, SpawnParameters);
			if(IsValid(RightHandEquipment))
			{
				RightHandEquipment->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHandSocketName);
				if(RightHandEquipment->GetEquipmentState() != EEquipmentStates::EquippedTwoHand)
				{
					RightHandEquipment->ServerSetEquipmentState(EEquipmentStates::EquippedRightHand);	
				}
				else
				{
					bMeleeLocomotion = RightHandEquipment->bMeleeLocomotion;
				}
			}
		}

		if(!IsValid(LeftHandEquipmentDefaultClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot spawn null left hand equipment class for %s."), *GetNameSafe(this));
		}
		else
		{
			LeftHandEquipment = World->SpawnActor<AEquipmentActor>(LeftHandEquipmentDefaultClass, Transform, SpawnParameters);
			if(IsValid(LeftHandEquipment))
			{
				LeftHandEquipment->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHandSocketName);

				if(LeftHandEquipment->GetEquipmentState() != EEquipmentStates::EquippedTwoHand)
				{
					LeftHandEquipment->ServerSetEquipmentState(EEquipmentStates::EquippedLeftHand);	
				}
				else
				{
					bMeleeLocomotion = LeftHandEquipment->bMeleeLocomotion;	
				}
			}	
		}
	}

	// On locally controlled characters, tick, set endurance, and allow kick.
	if(IsLocallyControlled())
	{
		Endurance = EnduranceMaximum;
		SetActorTickEnabled(true);

		KickCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ACeremonyCharacter::OnKickComponentOverlap);
	}
}

void ACeremonyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Destroy the equipment that was spawned on the server.
	if(HasAuthority())
	{
		if(IsValid(LeftHandEquipment))
		{
			LeftHandEquipment->Destroy();
		}

		if(IsValid(RightHandEquipment))
		{
			RightHandEquipment->Destroy();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ACeremonyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACeremonyCharacter, CosmeticAnimMontage, COND_SkipOwner);

	DOREPLIFETIME(ACeremonyCharacter, Health);

	DOREPLIFETIME_CONDITION(ACeremonyCharacter, bIsLockedOn, COND_SkipOwner);
	
	DOREPLIFETIME_CONDITION(ACeremonyCharacter, bIsRunning, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ACeremonyCharacter, bIsStaggered, COND_SkipOwner);
	
	DOREPLIFETIME_CONDITION(ACeremonyCharacter, LeftHandEquipment, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACeremonyCharacter, RightHandEquipment, COND_OwnerOnly);

	DOREPLIFETIME(ACeremonyCharacter, ClientPlayerNumber);

	DOREPLIFETIME_CONDITION(ACeremonyCharacter, bMeleeLocomotion, COND_SkipOwner);
}

void ACeremonyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("CameraPitch", this, &ACeremonyCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("CameraYaw", this, &ACeremonyCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACeremonyCharacter::Jump);

	PlayerInputComponent->BindAction("Kick", IE_Pressed, this, &ACeremonyCharacter::Kick);
	
	PlayerInputComponent->BindAction("LeftHandUse1", IE_Pressed, this, &ACeremonyCharacter::LeftHandPress1);
	PlayerInputComponent->BindAction("LeftHandUse2", IE_Pressed, this, &ACeremonyCharacter::LeftHandPress2);
	PlayerInputComponent->BindAction("LeftHandUse1", IE_Released, this, &ACeremonyCharacter::LeftHandRelease1);
	PlayerInputComponent->BindAction("LeftHandUse2", IE_Released, this, &ACeremonyCharacter::LeftHandRelease2);

	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &ACeremonyCharacter::LockOn);
	
	PlayerInputComponent->BindAction("RightHandUse1", IE_Pressed, this, &ACeremonyCharacter::RightHandPress1);
	PlayerInputComponent->BindAction("RightHandUse2", IE_Pressed, this, &ACeremonyCharacter::RightHandPress2);
	PlayerInputComponent->BindAction("RightHandUse1", IE_Released, this, &ACeremonyCharacter::RightHandRelease1);
	PlayerInputComponent->BindAction("RightHandUse2", IE_Released, this, &ACeremonyCharacter::RightHandRelease2);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ACeremonyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACeremonyCharacter::MoveRight);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ACeremonyCharacter::RunPress);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ACeremonyCharacter::RunRelease);
	
	// Set up the debug service.
	DebugComponent->BindActions(PlayerInputComponent);
}

void ACeremonyCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check for endurance recovery over time.
	if(bIsRunning && (!FMath::IsNearlyZero(MoveForwardLastValue) || !FMath::IsNearlyZero(MoveRightLastValue)))
	{
		float EnduranceDelta = RunEnduranceCostPerSecond * DeltaTime;
		if(Endurance - EnduranceDelta < 0.0f)
		{
			EnduranceDelta = Endurance - RunToZeroEndurancePenalty;
			bIsRunning = false;
			DebugComponent->UpdateCharacterStateText();
		}

		DepleteEndurance(EnduranceDelta);
	}
	else if(bAllowEnduranceRecovery && Endurance < EnduranceMaximum)
	{
		float EnduranceDelta = EnduranceRecoveryPerSecond * DeltaTime;
		if(bIsBlocking)
		{
			EnduranceDelta = BlockingEnduranceRecoveryPerSecond * DeltaTime;
		}
		else if(bIsAiming)
		{
			EnduranceDelta = AimingEnduranceRecoveryPerSecond * DeltaTime;
		}

		if(Endurance + EnduranceDelta > EnduranceMaximum)
		{
			EnduranceDelta = EnduranceMaximum - Endurance;
		}
		
		DepleteEndurance(EnduranceDelta * -1.0f);
	}
	
	// Check for forced movement with no control, see SetAnimMovement().
	if(bForcedMovement && !bAllowMovement)
	{
		AddMovementInput(ForcedMovementDirection, ForcedMovementRate);
	}

	// Check for stunned count down.
	if(StunTimer > 0.0f)
	{
		StunTimer -= DeltaTime;
		if(StunTimer <= 0.0f)
		{
			StunCount = 0;
			StunTimer = 0.0f;
			SetIsStunned(false);
			SetAllowMovement(true);
			StopMontageGlobally();	
		}
	}

	// Check for staggered count down.
	if(StaggerTimer > 0.0f)
	{
		StaggerTimer -= DeltaTime;
		if(StaggerTimer <= 0.0f)
		{
			// Stop stagger on the server, so it can replicate it to all clients.
			Server_SetIsStaggered(false);

			SetIsStaggered(false);
			SetAllowMovement(true);
			StopMontageGlobally();
		}
	}
}

#pragma region Client

void ACeremonyCharacter::Client_BackStabbed_Implementation()
{
	CancelActions();
	SetAllowMovement(false);
	SetIsInvincible(true);
	SetAllowEnduranceRecovery(false);
	PlayMontageGlobally(BackStabMontage);
	SetOnMontageEndedDelegate(this, "OnBackStabOrRiposteMontageEnded", BackStabMontage);
}

void ACeremonyCharacter::Client_DepleteEnduranceCanStagger_Implementation(const float EnduranceToDeplete)
{
	DepleteEndurance(EnduranceToDeplete);

	if(bIsBlocking)
	{
		AShieldActor* ShieldActor;
		if(bIsShieldLeftHanded)
		{
			ShieldActor = Cast<AShieldActor>(LeftHandEquipment);
		}
		else
		{
			ShieldActor = Cast<AShieldActor>(RightHandEquipment);
		}

		if(IsValid(ShieldActor) && Endurance > 0.0f)
		{
			ShieldActor->ShowBlockImpact();
		}
	}

	if(Endurance < 0.0f)
	{
		Client_Staggered();
	}
}

void ACeremonyCharacter::Client_Riposted_Implementation()
{
	CancelActions();
	SetIsStaggered(false);
	SetAllowMovement(false);
	SetIsInvincible(true);
	SetAllowEnduranceRecovery(false);
	PlayMontageGlobally(RiposteMontage);
	SetOnMontageEndedDelegate(this, "OnBackStabOrRiposteMontageEnded", RiposteMontage);
}

void ACeremonyCharacter::Client_Staggered_Implementation()
{
	LeftHandEquipment->CancelActions();
	RightHandEquipment->CancelActions();
	SetAllowMovement(false);
	SetIsStaggered(true);
	PlayMontageGlobally(StaggerMontage);
}

void ACeremonyCharacter::Client_Stunned_Implementation(const float InStunTime)
{
	if(GetIsStaggered())
	{
		// Stop stagger and become stunned.
		SetIsStaggered(false);
	}

	StunCount++;
	if(StunCount > StunCountMaximum)
	{
		SetIsStunned(false);
		StunCount = 0;
		SetAllowMovement(true);
		StopMontageGlobally();
	}
	else
	{
		StunTimer = InStunTime;

		// If already stunned, just continue to wait for client tick to stop the stun.
		if(bIsStunned)
		{
			return;
		}

		// Otherwise, play the stunned montage and set the flag.
		CancelActions();
		SetAllowMovement(false);
		SetIsStunned(true);
		PlayMontageGlobally(StunMontage);
	}
}

void ACeremonyCharacter::OnRep_ClientPlayerNumber() const
{
	// Set player color
	if(ClientPlayerNumber > 0)
	{
		UMaterialInstanceDynamic* Dynamic1 = GetMesh()->CreateDynamicMaterialInstance(0);
		UMaterialInstanceDynamic* Dynamic2 = GetMesh()->CreateDynamicMaterialInstance(1);
		
		switch(ClientPlayerNumber)
		{
		case 1:
			Dynamic1->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor(0.5f, 0.05f, 0.05f));
			Dynamic2->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor(0.5f, 0.05f, 0.05f));
			break;
		case 2:
			Dynamic1->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor(0.05f, 0.5f, 0.05f));
			Dynamic2->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor(0.05f, 0.5f, 0.05f));
			break;
		default:
			Dynamic1->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor(0.05f, 0.05f, 0.5f));
			Dynamic2->SetVectorParameterValue(TEXT("BodyColor"), FLinearColor(0.05f, 0.05f, 0.5f));
			break;
		}	
	}
}

#pragma endregion

#pragma region Equipment

void ACeremonyCharacter::CancelBlocking()
{
	if(bIsShieldLeftHanded)
	{
		LeftHandEquipment->CancelActions();	
	}
	else
	{
		RightHandEquipment->CancelActions();
	}
	
	SetIsBlocking(false);
}

void ACeremonyCharacter::CheckForAttackTransition(const bool bIsRightHanded) const
{
	if(bIsRightHanded)
	{
		if(IsValid(RightHandEquipment))
		{
			RightHandEquipment->CheckForAttackTransition();
		}
	}
	else
	{
		if(IsValid(LeftHandEquipment))
		{
			LeftHandEquipment->CheckForAttackTransition();
		}
	}
}

bool ACeremonyCharacter::GetCanAttack() const
{
	return (Endurance > 0.0f) && !bIsKicking && !bIsParrying && !bIsStunned && !bIsStaggered && !bIsRolling && !GetCharacterMovement()->IsFalling();
}

bool ACeremonyCharacter::GetCanBlock() const
{
	return Endurance > 0.0f && !bIsAttacking && !bIsKicking && !bIsStunned && !bIsParrying && !bIsStaggered && !bIsRolling;
}

void ACeremonyCharacter::LeftHandPress1()
{
	if(IsValid(LeftHandEquipment))
	{
		LeftHandEquipment->Press1();
	}
	else if(IsValid(RightHandEquipment) && RightHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		RightHandEquipment->TwoHandPress1();
	}
	
	bLeftHandPress1HeldDown = true;
}

void ACeremonyCharacter::LeftHandPress2()
{
	if(IsValid(LeftHandEquipment))
	{
		LeftHandEquipment->Press2();
	}
	else if(IsValid(RightHandEquipment) && RightHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		RightHandEquipment->TwoHandPress2();
	}

	bLeftHandPress2HeldDown = true;
}

void ACeremonyCharacter::LeftHandRelease1()
{
	if(IsValid(LeftHandEquipment))
	{
		LeftHandEquipment->Release1();
	}
	else if(IsValid(RightHandEquipment) && RightHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		RightHandEquipment->TwoHandRelease1();
	}
	
	bLeftHandPress1HeldDown = false;
}

void ACeremonyCharacter::LeftHandRelease2()
{
	if(IsValid(LeftHandEquipment))
	{
		LeftHandEquipment->Release2();
	}
	else if(IsValid(RightHandEquipment) && RightHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		RightHandEquipment->TwoHandRelease2();
	}
	
	bLeftHandPress2HeldDown = false;
}

void ACeremonyCharacter::RightHandPress1()
{
	if(IsValid(RightHandEquipment))
	{
		RightHandEquipment->Press1();
	}
	else if(IsValid(LeftHandEquipment) && LeftHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		LeftHandEquipment->TwoHandPress1();
	}
	
	bRightHandPress1HeldDown = true;
}

void ACeremonyCharacter::RightHandPress2()
{
	if(IsValid(RightHandEquipment))
	{
		RightHandEquipment->Press2();
	}
	else if(IsValid(LeftHandEquipment) && LeftHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		LeftHandEquipment->TwoHandPress2();
	}
		
	bRightHandPress2HeldDown = true;
}

void ACeremonyCharacter::RightHandRelease1()
{
	if(IsValid(RightHandEquipment))
	{
		RightHandEquipment->Release1();
	}
	else if(IsValid(LeftHandEquipment) && LeftHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		LeftHandEquipment->TwoHandRelease1();
	}
		
	bRightHandPress1HeldDown = false;
}

void ACeremonyCharacter::RightHandRelease2()
{
	if(IsValid(RightHandEquipment))
	{
		RightHandEquipment->Release2();
	}
	else if(IsValid(LeftHandEquipment) && LeftHandEquipment->GetEquipmentState() == EEquipmentStates::EquippedTwoHand)
	{
		LeftHandEquipment->TwoHandRelease2();
	}
		
	bRightHandPress2HeldDown = false;	
}

void ACeremonyCharacter::SetAttackCanDamage(const bool bRightHand, const bool bIsActive) const
{
	if(bRightHand)
	{
		if(IsValid(RightHandEquipment))
		{
			RightHandEquipment->SetAttackCanDamage(bIsActive);

			// Show collision on the weapon if showing collision capsule.
			if(IsShowingDebugCollision())
			{
				RightHandEquipment->ShowCollision(bIsActive);
			}
		}
	}
	else
	{
		if(IsValid(LeftHandEquipment))
		{
			LeftHandEquipment->SetAttackCanDamage(bIsActive);

			// Show collision on the weapon if showing collision capsule.
			if(IsShowingDebugCollision())
			{
				LeftHandEquipment->ShowCollision(bIsActive);
			}
		}
	}
	
	DebugComponent->SetAttackCanDamageText(bIsActive);
}

void ACeremonyCharacter::SetIsAiming(const bool bAiming)
{
	bIsAiming = bAiming;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetIsAttacking(const bool bAttacking)
{
	bIsAttacking = bAttacking;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetIsBlocking(const bool bBlocking, const bool bIsLeftHanded)
{
	bIsBlocking = bBlocking;
	bIsShieldLeftHanded = bIsLeftHanded;
	DebugComponent->UpdateCharacterStateText();

	if(GetLocalRole() < ROLE_Authority)
	{
		// The server needs to know if the character is blocking for ServerVerifyOverlapForDamage.
		Server_SetIsBlocking(bBlocking);
	}
}

void ACeremonyCharacter::SetIsParrying(const bool bParry)
{
	bIsParrying = bParry;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetParryCanStagger(const bool bCanStagger)
{
	bParryCanStagger = bCanStagger;
	DebugComponent->UpdateCharacterStateText();

	if(GetLocalRole() < ROLE_Authority)
	{
		// The server needs to know if the character is in active parry frames for ServerVerifyOverlapsForDamage.
		Server_SetParryCanStagger(bCanStagger);
	}
}

#pragma endregion 

#pragma region Gameplay

void ACeremonyCharacter::CancelActions()
{
	if(IsValid(LeftHandEquipment))
	{
		LeftHandEquipment->CancelActions();
	}

	if(IsValid(RightHandEquipment))
	{
		RightHandEquipment->CancelActions();	
	}

	SetIsKicking(false);
	SetIsParrying(false);
	SetIsRolling(false);
	SetIsRunning(false);
	SetIsStaggered(false);
	SetIsStunned(false);
}

void ACeremonyCharacter::CheckForResumingAction()
{
	if(bRollOnResumeAction)
	{
		Roll();
		return;
	}

	if(bKickOnResumeAction)
	{
		Kick();
		return;
	}

	if(IsValid(LeftHandEquipment))
	{
		LeftHandEquipment->Resume1(bLeftHandPress1HeldDown);
		
		LeftHandEquipment->Resume2(bLeftHandPress2HeldDown);
	}
	
	if(IsValid(RightHandEquipment))
	{
		RightHandEquipment->Resume1(bRightHandPress1HeldDown);

		RightHandEquipment->Resume2(bRightHandPress2HeldDown);
	}

	if(bRunHeldDown)
	{
		RunPress();
	}
}

bool ACeremonyCharacter::GetCanPerformStandardAction() const
{
	return (Endurance > 0.0f) && !bIsKicking && !bIsParrying && !bIsStunned && !bIsStaggered && !bIsRolling && !bIsAttacking && !GetCharacterMovement()->IsFalling();
}

void ACeremonyCharacter::DepleteEndurance(const float EnduranceChange)
{
	ACeremonyPlayerController* CeremonyController = Cast<ACeremonyPlayerController>(Controller);
	if(IsValid(CeremonyController))
	{
		UCeremonyUserWidget* Widget = CeremonyController->CharacterWidget;
		if(IsValid(Widget))
		{
			Endurance -= EnduranceChange;
			Widget->OnEnduranceChanged(Endurance, EnduranceMaximum);	
		}
	}
}

bool ACeremonyCharacter::IsShowingDebugCollision() const
{
	return DebugComponent->bEnableCollisionDebug;
}

void ACeremonyCharacter::PlayFootstepSound(const bool bRightFoot)
{
	const FName FootBoneName = bRightFoot? InverseKinematicsComponent->GetRightFootBoneName() : InverseKinematicsComponent->GetLeftFootBoneName();
	USoundBase* Sound = FootstepComponent->GetFootstepSound(FootBoneName);
	if(Sound != nullptr)
	{
		Server_PlaySound(Sound);
	}
}

void ACeremonyCharacter::PlaySound(USoundBase* Sound) const
{
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());
}

void ACeremonyCharacter::OnRep_Health() const
{
	if(IsLocallyControlled())
	{
		ACeremonyPlayerController* CeremonyController = Cast<ACeremonyPlayerController>(Controller);
		if(IsValid(CeremonyController))
		{
			UCeremonyUserWidget* Widget = CeremonyController->CharacterWidget;
			if(IsValid(Widget))
			{
				Widget->OnHealthChanged(Health, HealthMaximum);	
			}
		}
	}
	else
	{
		UCeremonyOpponentUserWidget* OpWidget = Cast<UCeremonyOpponentUserWidget>(OpponentWidget->GetUserWidgetObject());
		if(IsValid(OpWidget))
		{
			OpWidget->OnHealthChanged(Health, HealthMaximum);
		}
	}
}

void ACeremonyCharacter::SetAllowEnduranceRecovery(const bool bAllowRecovery)
{
	bAllowEnduranceRecovery = bAllowRecovery;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetIsInvincible(const bool bInvincible)
{
	bIsInvincible = bInvincible;
	DebugComponent->UpdateCharacterStateText();

	if(GetLocalRole() < ROLE_Authority)
	{
		// The server needs to know if the character is invincible for ServerVerifyOverlapForDamage.
		Server_SetIsInvincible(bInvincible);
	}
}

void ACeremonyCharacter::SetIsStaggered(const bool bStaggered)
{
	if(bStaggered)
	{
		StaggerTimer = StaggerTime;		
	}
	else
	{
		StaggerTimer = 0.0f;
	}

	bIsStaggered = bStaggered;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetIsStunned(const bool bStunned)
{
	bIsStunned = bStunned;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetOpponentHasLockedOn(const bool bHasLockedOn) const
{
	LockOnWidget->SetVisibility(bHasLockedOn);

	UCeremonyOpponentUserWidget* UserWidget = Cast<UCeremonyOpponentUserWidget>(OpponentWidget->GetUserWidgetObject());
	if(IsValid(UserWidget))
	{
		UserWidget->SetHealthBarStayVisible(bHasLockedOn);	
	}
}

#pragma endregion

#pragma region Kick

void ACeremonyCharacter::Kick()
{
	if(GetCanPerformStandardAction())
	{
		bKickOnResumeAction = false;
		
		if(bIsBlocking)
		{
			CancelBlocking();
		}

		if(bIsRunning)
		{
			SetIsRunning(false);
		}

		KickedActors.Empty();
		
		DepleteEndurance(KickEnduranceConsumption);
		SetAllowEnduranceRecovery(false);
		SetAllowMovement(false);
		PlayMontageGlobally(KickMontage);
		SetOnMontageEndedDelegate(this, "OnKickMontageComplete", KickMontage);

		SetIsKicking(true);
	}
	else
	{
		bKickOnResumeAction = true;
	}
}

void ACeremonyCharacter::OnKickComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const UWorld* World = GetWorld();
	
	// Prevent kicking a character more than once with the same kick.
	if(KickedActors.Contains(OtherActor) || !IsValid(World))
	{
		return;
	}

	// They have overlapped, we need to get the location of the overlap by sweeping a small distance.
	TArray<FHitResult> OutHits;

	const FVector Start = KickCapsuleComponent->GetComponentLocation();
	const FVector End = FVector(Start.X + 1.0f, Start.Y + 1.0f, Start.Z + 1.0f);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActors(KickedActors);
	
	if(World->SweepMultiByObjectType(OutHits, Start, End, KickCapsuleComponent->GetComponentRotation().Quaternion(), ObjectQueryParams, KickCapsuleComponent->GetCollisionShape(), Params))
	{
		for(auto OutHit : OutHits)
		{
			if(OutHit.Actor == OtherActor && OutHit.GetComponent() == OtherComp)
			{
				if(IsShowingDebugCollision())
				{
					DrawDebugCapsule(World, Start, KickCapsuleComponent->GetScaledCapsuleHalfHeight(),
					                 KickCapsuleComponent->GetScaledCapsuleRadius(),
					                 KickCapsuleComponent->GetComponentRotation().Quaternion(), FColor::Red, false, 3.0f, 0, 0);
					
					DrawDebugSphere(World, OutHit.ImpactPoint, 3.0f, 8, FColor::Red, false, 3.0f, 0, 1);
				}

				KickedActors.Add(OtherActor);

				ACeremonyCharacter* CharacterHit = Cast<ACeremonyCharacter>(OutHit.GetActor());
				
				const FVector Packed = FVector(0.0f, KickEnduranceDamage, 0.0f);
				Server_VerifyOverlapForDamage(CharacterHit, OutHit.ImpactPoint, Packed, static_cast<uint8>(EDamageTypes::Kick));
				break;
			}
		}
	}	
}

void ACeremonyCharacter::OnKickMontageComplete(UAnimMontage* Montage, const bool bInterrupted)
{
	ClearOnMontageEndedDelegate();
	SetAllowEnduranceRecovery(true);
	SetAllowMovement(true);
	SetIsKicking(false);
	CheckForResumingAction();

	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetIsKicking(const bool bKick)
{
	bIsKicking = bKick;

	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetKickCanDamage(const bool bCanDamage) const
{
	KickCapsuleComponent->SetGenerateOverlapEvents(bCanDamage);

	if(IsShowingDebugCollision())
	{
		KickCapsuleComponent->SetHiddenInGame(!bCanDamage);
	}
	DebugComponent->SetKickCanDamageText(bCanDamage);
}

#pragma endregion 

#pragma region Montage

void ACeremonyCharacter::ClearOnMontageEndedDelegate() const
{
	const USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if(IsValid(SkeletalMesh))
	{
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
		if(IsValid(AnimInstance))
		{
			FOnMontageEnded EmptyDelegate;
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate);
		}
	}
}

void ACeremonyCharacter::OnBackStabOrRiposteMontageEnded(UAnimMontage* Montage, const bool bInterrupted)
{
	SetAllowMovement(true);
	SetIsInvincible(false);
	SetAllowEnduranceRecovery(true);
	StopMontageGlobally();
	ClearOnMontageEndedDelegate();
}

void ACeremonyCharacter::OnRepCosmeticAnimMontage() const
{
	const USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if(IsValid(SkeletalMesh))
	{
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
		if(IsValid(AnimInstance))
		{
			if(IsValid(CosmeticAnimMontage.Montage))
			{
				AnimInstance->Montage_Play(CosmeticAnimMontage.Montage, 1.0f, EMontagePlayReturnType::MontageLength, CosmeticAnimMontage.Position);
			}
			else
			{
				AnimInstance->Montage_Stop(MontageBlendOutTime);
			}
		}
	}
}

float ACeremonyCharacter::PlayMontage(UAnimMontage* MontageToPlay, const FName JumpToSection) const
{
	const USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if(IsValid(SkeletalMesh))
	{
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
		if(IsValid(AnimInstance))
		{
			AnimInstance->Montage_Play(MontageToPlay);

			if(JumpToSection != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(JumpToSection);
			}

			return AnimInstance->Montage_GetPosition(MontageToPlay);
		}
	}

	return 0.0f;
}

void ACeremonyCharacter::PlayMontageGlobally(UAnimMontage* MontageToPlay, const FName JumpToSection)
{
	const float Position = PlayMontage(MontageToPlay, JumpToSection);
	Server_PlayCosmeticAnimMontage(MontageToPlay, Position);
}

void ACeremonyCharacter::SetOnMontageEndedDelegate(UObject* UserObject, FName FunctionName,
	UAnimMontage* ActiveMontage) const
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if(IsValid(SkeletalMesh))
	{
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
		if(IsValid(AnimInstance))
		{
			FOnMontageEnded Delegate = FOnMontageEnded::CreateUFunction(UserObject, FunctionName);
			AnimInstance->Montage_SetEndDelegate(Delegate, ActiveMontage);
		}
	}
}

void ACeremonyCharacter::StopMontageGlobally()
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if(IsValid(SkeletalMesh))
	{
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
		if(IsValid(AnimInstance))
		{
			AnimInstance->Montage_Stop(MontageBlendOutTime);
			Server_PlayCosmeticAnimMontage(nullptr, 0);
		}
	}
}

#pragma endregion 

#pragma region Movement

void ACeremonyCharacter::AddControllerPitchInput(const float Val)
{
	if(FMath::IsNearlyZero(Val))
	{
		return;
	}

	if(!bIsLockedOn)
	{
		Super::AddControllerPitchInput(Val);
	}
}

void ACeremonyCharacter::AddControllerYawInput(const float Val)
{
	if(FMath::IsNearlyZero(Val))
	{
		return;
	}

	if(bIsLockedOn)
	{
		// While locked on, yaw input will force selecting different targets.
		LockOnComponent->YawInput(Val);
	}
	else
	{
		Super::AddControllerYawInput(Val);
	}
}

void ACeremonyCharacter::AddControllerYawInputFromLockOn(const float Val)
{
	Super::AddControllerYawInput(Val);
}

void ACeremonyCharacter::GetForcedMovement(bool& bOutForcedMovement, float& OutForcedMovementRate,
                                           FVector& OutForcedMovementDirection) const
{
	bOutForcedMovement = bForcedMovement;
	OutForcedMovementRate = ForcedMovementRate;
	OutForcedMovementDirection = ForcedMovementDirection;
}

void ACeremonyCharacter::Jump()
{
	if(!GetCharacterMovement()->IsFalling() && Endurance > 0.0f)
	{
		DepleteEndurance(JumpEnduranceConsumption);
		Super::Jump();
	}
}

void ACeremonyCharacter::LockOn()
{
	LockOnComponent->Press();
}

void ACeremonyCharacter::MoveForward(float AxisValue)
{
	if(!FMath::IsNearlyZero(AxisValue) && bAllowMovement)
	{
		const FRotator Rotation = FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);

		if(bForcedMovement && FMath::Abs(AxisValue) > FMath::Abs(ForcedMovementRate))
		{
			if((AxisValue > 0.0 && ForcedMovementRate > 0.0) ||
				(AxisValue < 0.0 && ForcedMovementRate < 0.0))
			{
				// Same sign, just limit the value
				AxisValue = ForcedMovementRate;	
			}
			else
			{
				// Different sign.
				AxisValue = -ForcedMovementRate;
			}
		}
		
		AddMovementInput(Direction, AxisValue);
	}

	MoveForwardLastValue = AxisValue;
}

void ACeremonyCharacter::MoveRight(float AxisValue)
{
	if(!FMath::IsNearlyZero(AxisValue) && bAllowMovement)
	{
		const FRotator Rotation = FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

		if(bForcedMovement && FMath::Abs(AxisValue) > FMath::Abs(ForcedMovementRate))
		{
			if((AxisValue > 0.0 && ForcedMovementRate > 0.0) ||
				(AxisValue < 0.0 && ForcedMovementRate < 0.0))
			{
				// Same sign, just limit the value
				AxisValue = ForcedMovementRate;	
			}
			else
			{
				// Different sign.
				AxisValue = -ForcedMovementRate;
			}
		}
		
		AddMovementInput(Direction, AxisValue);
	}

	MoveRightLastValue = AxisValue;
}

void ACeremonyCharacter::RunPress()
{
	if(Endurance < 0.0f)
	{
		return;
	}

	SetIsRunning(true);

	bRunHeldDown = true;

	// Capture when it was pressed to trigger rolling.
	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		RollTimeStamp = World->GetTimeSeconds();
	}
}

void ACeremonyCharacter::RunRelease()
{
	SetIsRunning(false);

	bRunHeldDown = false;

	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		if((World->GetTimeSeconds() - RollTimeStamp) < RollPressReleaseTime)
		{
			Roll();
		}		
	}
}

void ACeremonyCharacter::OnRep_IsLockedOn() const
{
	GetCharacterMovement()->bOrientRotationToMovement = !bIsLockedOn;
}

void ACeremonyCharacter::SetAllowMovement(const bool bAllow)
{
	bAllowMovement = bAllow;
	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetAnimMovement(const bool bInForcedMovement, const float InForcedMovementRate, const EMovementType UnlockedType,
	const EMovementType LockedType)
{
	if(bInForcedMovement)
	{
		bForcedMovement = true;
		ForcedMovementRate = InForcedMovementRate;

		const FRotator Rotation = FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector ForwardDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		
		switch(UnlockedType)
		{
		case EMovementType::ContinuousInputDirection:
			// Allow moving in any direction.
			bAllowMovement = true;
			break;
		case EMovementType::ForcedForwardOrBack:
			ForcedMovementDirection = ForwardDirection;
			break;
		case EMovementType::ForcedLastInputDirection:
			const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

			if(FMath::IsNearlyZero(MoveForwardLastValue) && FMath::IsNearlyZero(MoveRightLastValue))
			{
				ForcedMovementDirection = GetActorForwardVector();
			}
			else
			{
				ForcedMovementDirection =  (ForwardDirection * MoveForwardLastValue + RightDirection * MoveRightLastValue).GetSafeNormal();	
			}
			break;
		}
	}
	else
	{
		bForcedMovement = false;
		ForcedMovementRate = 0.0f;
		ForcedMovementDirection = FVector::ZeroVector;
	}

	DebugComponent->UpdateCharacterStateText();
}

void ACeremonyCharacter::SetIsLockedOn(const bool bLocked)
{
	bIsLockedOn = bLocked;
	
	if(bLocked)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	if(IsLocallyControlled())
	{
		DebugComponent->UpdateCharacterStateText();			
	}

	if(GetLocalRole() < ROLE_Authority)
	{
		// Set the locked on value on the server; it must replicate to all clients, so when animating the character will play proper animations.
		Server_SetIsLockedOn(bLocked);
	}
}

void ACeremonyCharacter::SetIsRunning(const bool bRun)
{
	if(bIsRunning != bRun)
	{
		bIsRunning = bRun;
		DebugComponent->UpdateCharacterStateText();

		// Need to replicate if this character has started running to all clients.
		if(GetLocalRole() < ROLE_Authority)
		{
			Server_SetIsRunning(bIsRunning);
		}
	}
}

#pragma endregion

#pragma region Multicast

void ACeremonyCharacter::Multicast_PlaySound_Implementation(USoundBase* Sound)
{
	PlaySound(Sound);
}

void ACeremonyCharacter::Multicast_KillCharacter_Implementation(ACeremonyCharacter* CharacterToKill)
{
	CharacterToKill->GetCharacterMovement()->StopMovementImmediately();
	CharacterToKill->SetActorTickEnabled(false);
	CharacterToKill->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterToKill->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CharacterToKill->GetMesh()->SetSimulatePhysics(true);
}

void ACeremonyCharacter::Multicast_SetActorRotation_Implementation(const FRotator Rotation)
{
	SetActorRotation(Rotation);
}

void ACeremonyCharacter::Multicast_SetOpponentWidgetDamage_Implementation(const float Damage)
{
	if(!IsLocallyControlled())
	{
		UCeremonyOpponentUserWidget* OpWidget = Cast<UCeremonyOpponentUserWidget>(OpponentWidget->GetUserWidgetObject());
		if(IsValid(OpWidget))
		{
			OpWidget->OnDamageChanged(Damage);
		}
	}
}

#pragma endregion

#pragma region Roll

void ACeremonyCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	SetAllowEnduranceRecovery(true);
	SetAllowMovement(true);
	StopMontageGlobally();
	ClearOnMontageEndedDelegate();
	SetIsRolling(false);

	CheckForResumingAction();
}

void ACeremonyCharacter::Roll()
{
	if(GetCanPerformStandardAction())
	{
		bRollOnResumeAction = false;

		if(bIsBlocking)
		{
			CancelBlocking();
		}

		DepleteEndurance(RollEnduranceConsumption);

		SetAllowEnduranceRecovery(false);
		SetAllowMovement(false);
		PlayMontageGlobally(RollMontage);
		SetOnMontageEndedDelegate(this, "OnRollMontageEnded", RollMontage);

		SetIsRolling(true);
	}
	else
	{
		bRollOnResumeAction = true;
	}
}

void ACeremonyCharacter::SetIsRolling(const bool bRoll)
{
	bIsRolling = bRoll;
	DebugComponent->UpdateCharacterStateText();
}

#pragma endregion

#pragma region Server

void ACeremonyCharacter::Server_SetActorRotation_Implementation(const FRotator NewRotation)
{
	Multicast_SetActorRotation(NewRotation);
}

void ACeremonyCharacter::Server_HelperKillCharacter(ACeremonyCharacter* Character)
{
	// Kill character on the server.
	Character->GetCharacterMovement()->StopMovementImmediately();
	Character->SetActorTickEnabled(false);
	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Character->GetMesh()->SetSimulatePhysics(true);
	Character->RightHandEquipment->Destroy();
	Character->LeftHandEquipment->Destroy();
	Character->SetLifeSpan(5.0f);
	
	// Kill on all clients.
	Multicast_KillCharacter(Character);
}

void ACeremonyCharacter::Server_PlayCosmeticAnimMontage_Implementation(UAnimMontage* MontageToPlay, const float Position)
{
	CosmeticAnimMontage.Set(MontageToPlay, Position);

	// On the listen server, force it to act like it replicated the client's animations, even though it didn't because it's the server.
	if(GetNetMode() == NM_ListenServer)
	{
		OnRepCosmeticAnimMontage();
	}
}

void ACeremonyCharacter::Server_VerifyBackStab_Implementation(ACeremonyCharacter* CharacterHit, const float Damage)
{
	if(!IsValid(CharacterHit) || CharacterHit->GetIsInvincible())
	{
		return;
	}

	const float DotProduct = FVector::DotProduct(GetActorForwardVector(), CharacterHit->GetActorForwardVector());
	const float Distance = FVector::Distance(GetActorLocation(), CharacterHit->GetActorLocation());

	if(Distance < ServerBackStabMaxDistance && DotProduct > ServerBackStabMinDotProduct)
	{
		CharacterHit->Multicast_SetOpponentWidgetDamage(Damage);
		CharacterHit->Health = FMath::Clamp(CharacterHit->Health - Damage, 0.0f, CharacterHit->HealthMaximum);
		
		if(CharacterHit->GetNetMode() == NM_ListenServer)
		{
			// Listen server will need to force an update, because it will not call OnRep locally when things change.
			CharacterHit->OnRep_Health();
		}

		if(CharacterHit->Health == 0.0f)
		{
			Server_HelperKillCharacter(CharacterHit);
		}
		else
		{
			CharacterHit->Client_BackStabbed();
		}
	}
}

void ACeremonyCharacter::Server_VerifyOverlapForDamage_Implementation(ACeremonyCharacter* CharacterHit, const FVector_NetQuantize ImpactPoint, const FVector_NetQuantize DamageEnduranceDamageStunTime, const uint8 IntDamageType)
{
	const UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to get world on %s."), *GetNameSafe(this));
		return;
	}

	TArray<FOverlapResult> OutOverlaps;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if(IsShowingDebugCollision())
	{
		DrawDebugSphere(World, ImpactPoint, ServerVerifyOverlapsSphereRadius, 8, FColor::Blue, false, 3.0f, 0, 0);

		if(IsValid(CharacterHit))
		{
			DrawDebugCapsule(World, CharacterHit->GetActorLocation(),
			                 CharacterHit->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			                 CharacterHit->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			                 CharacterHit->GetActorRotation().Quaternion(), FColor::Blue, false, 3.0f, 0, 0);
		}
	}

	if(IsValid(World) && World->OverlapMultiByObjectType(OutOverlaps, ImpactPoint, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(ServerVerifyOverlapsSphereRadius), Params))
	{
		for(FOverlapResult& Overlap : OutOverlaps)
		{
			if(Overlap.GetActor() == CharacterHit)
			{
				const float Damage = DamageEnduranceDamageStunTime.X;
				const float EnduranceDamage = DamageEnduranceDamageStunTime.Y;
				const float StunTime = DamageEnduranceDamageStunTime.Z;
				const EDamageTypes DamageType = static_cast<EDamageTypes>(IntDamageType);
				
				// Check if the character is in invincibility frames, prevent damage.
				if(CharacterHit->GetIsInvincible())
				{
					return;
				}

				if(CharacterHit->GetParryCanStagger())
				{
					if(DamageType == EDamageTypes::Kick)
					{
						// If the character is parrying and receives a kick, they just take endurance damage with no absorption.
						CharacterHit->Client_DepleteEnduranceCanStagger(EnduranceDamage);						
						Multicast_PlaySound(KickInterruptSound);
						return;
					}

					// Set the character that is attacking to staggered, the target is parrying.

					// On the server, set stagger on the attacking client (to replicate to all clients).
					bIsStaggered = true;

					// Trigger stagger on the attacking client
					Client_Staggered();

					// Play parry sound on all clients.
					Multicast_PlaySound(ParrySound);

					return;
				}

				if(CharacterHit->GetIsBlocking())
				{
					// Check if the character is blocking
					AShieldActor* ShieldActor;
					if(CharacterHit->GetIsBlocking() && CharacterHit->GetIsShieldLeftHanded())
					{
						ShieldActor = Cast<AShieldActor>(LeftHandEquipment);
					}
					else
					{
						ShieldActor = Cast<AShieldActor>(RightHandEquipment);
					}
										
					if(IsValid(ShieldActor))
					{
						float OutDamage;
						float OutEnduranceDamage;
						ShieldActor->GetDamageAfterAbsorption(Damage, DamageType, EnduranceDamage, OutDamage, OutEnduranceDamage);

						if(DamageType == EDamageTypes::Kick)
						{
							Multicast_PlaySound(KickBlockedSound);
						}
						else
						{
							Multicast_PlaySound(BlockAttackSound);
						}

						CharacterHit->Multicast_SetOpponentWidgetDamage(OutDamage);
						CharacterHit->Health = FMath::Clamp(CharacterHit->Health - OutDamage, 0.0f, CharacterHit->HealthMaximum);
						
						if(GetNetMode() == NM_ListenServer)
						{
							// Listen server will need to force an update, because it will not call OnRep locally.
							CharacterHit->OnRep_Health();
						}
						
						CharacterHit->Client_DepleteEnduranceCanStagger(OutEnduranceDamage);
					}
				}
				else if(DamageType == EDamageTypes::Kick)
				{
					// Interrupt the character that it connected with.
					CharacterHit->Client_Stunned(KickStunTime);

					Multicast_PlaySound(KickInterruptSound);
				}
				else
				{
					Multicast_PlaySound(AttackHitSound);

					CharacterHit->Multicast_SetOpponentWidgetDamage(Damage);
					CharacterHit->Health = FMath::Clamp(CharacterHit->Health - Damage, 0.0f, CharacterHit->HealthMaximum);

					if(GetNetMode() == NM_ListenServer)
					{
						// Listen server will need to force an update, because it will not call OnRepHealth when health changes, so the bar won't update.
						CharacterHit->OnRep_Health();
					}
					
					if(CharacterHit->Health > 0.0f)
					{
						CharacterHit->Client_Stunned(StunTime);	
					}
				}

				if(CharacterHit->Health == 0.0f)
				{
					Server_HelperKillCharacter(CharacterHit);
				}
				break;
			}
		}
	}
}

bool ACeremonyCharacter::Server_VerifyOverlapForDamage_Validate(ACeremonyCharacter* CharacterHit, FVector_NetQuantize ImpactPoint, const FVector_NetQuantize DamageEnduranceDamageStunTime, uint8 DamageType)
{
	const float DamageStandard = DamageEnduranceDamageStunTime.X;
	const float EnduranceDamage = DamageEnduranceDamageStunTime.Y;

	if(DamageStandard < 0.0f || EnduranceDamage < 0.0f)
	{
		return false;
	}

	return true;
}

void ACeremonyCharacter::Server_VerifyRiposte_Implementation(ACeremonyCharacter* CharacterHit, const float Damage)
{
	if(!IsValid(CharacterHit) || CharacterHit->GetIsInvincible())
	{
		return;
	}

	const float DotProduct = FVector::DotProduct(GetActorForwardVector(), CharacterHit->GetActorForwardVector());
	const float Distance = FVector::Distance(GetActorLocation(), CharacterHit->GetActorLocation());

	if(Distance < ServerRiposteMaxDistance && DotProduct < ServerRiposteMaxDotProduct)
	{
		CharacterHit->Multicast_SetOpponentWidgetDamage(Damage);
		CharacterHit->Health = FMath::Clamp(CharacterHit->Health - Damage, 0.0f, CharacterHit->HealthMaximum);

		if(CharacterHit->GetNetMode() == NM_ListenServer)
		{
			// Listen server will need to force an update, because it will not call OnRepHealth when health changes, so the bar won't update.
			CharacterHit->OnRep_Health();
		}

		if(CharacterHit->Health == 0.0f)
		{
			Server_HelperKillCharacter(CharacterHit);
		}
		else
		{
			CharacterHit->Client_Riposted();
		}
	}
}

#pragma endregion

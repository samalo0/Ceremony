// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugComponent.generated.h"

class ACeremonyCharacter;

/**
 * Base class for the debug component, which displays state information and commands for debug viewing.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CEREMONY_API UDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UDebugComponent();

	void BeginPlay() override;

	void BindActions(class UInputComponent* InputComponent);
	
	void SetAttackCanDamageText(const bool bCanDamage) const;

	void SetKickCanDamageText(const bool bCanDamage) const;
	
	void ToggleShowIKTraces();

	void UpdateCharacterStateText() const;

	bool bEnableCollisionDebug = false;
	
private:	

	void DebugToggle();

	void ToggleShowCollision();
	
	void ToggleSlowMotion();
	
	// Reference to the debug widget.
	UPROPERTY(Transient)
	class UDebugUserWidget* DebugUserWidget;
	
	// Reference to the owner character.
	UPROPERTY(Transient)
	ACeremonyCharacter* OwnerCharacter;
	
};

// Copyright 2020 Stephen Maloney

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

class ACeremonyCharacter;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class CEREMONY_API AProjectileActor : public AActor
{

	GENERATED_BODY()
	
public:	

	AProjectileActor();

	void Tick(float DeltaSeconds) override;
	
protected:

	void BeginPlay() override;

	UFUNCTION()
	void OnSphereComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	// Owner reference.
	UPROPERTY(Transient)
	ACeremonyCharacter* OwnerCharacter;

	// Show debug messages/traces.
	UPROPERTY(EditDefaultsOnly)
	bool bShowDebug = true;
	
#pragma region Components

protected:
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComponent;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
#pragma endregion
	
};

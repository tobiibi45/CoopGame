// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Moves character forward
	void MoveForward(float Value);

	//Moves character Backwards
	void MoveRight(float Value);

	//Allows the character to crouch
	void BeginCrouch();

	//Stops the character from crouching
	void EndCrouch();

	//Attaches a camera to the player for the third person controller mode
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0, ClampMax = 100))
	float  ZoomInterpSpeed;

	/*Default FOV during get play*/
	float DefaultFOV;

	/*Allows the player to zoom in with weapon*/
	void BeginZoom();

	/*Allows the player to stop zooming in with weapon*/
	void EndZoom();

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	/*Pointer class to weapon*/
	ASWeapon* CurrentWeapon;

	//Sub class of damage that returns the type of damage that is caused
	UPROPERTY(EditDefaultsOnly,Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	/*Allows the player to fire weapon*/
	void StartFire();

	/*Stops player from firing weapon*/
	void StopFire();

	/*Stops player from firing weapon*/
	void ReloadWeapon();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/*Pawn Died Previously*/
	UPROPERTY(BlueprintReadOnly, Category="Player")
	bool bDied;

	/*Can reload Previously*/
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsReloading;

	void SetIsReloading();

	FTimerHandle TimerHandle_TimeTOReload;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called to get the eye location for the character
	virtual FVector GetPawnViewLocation() const override;

};

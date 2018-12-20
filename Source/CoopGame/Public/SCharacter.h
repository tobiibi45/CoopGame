// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:

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

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0, ClampMax = 100))
	float  ZoomInterpSpeed;

	/*Default FOV during get play*/
	float DefaultFOV;

	void BeginZoom();

	void EndZoom();

	ASWeapon* Cu
public:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called to get the eye location for the character
	virtual FVector GetPawnViewLocation() const override;

};

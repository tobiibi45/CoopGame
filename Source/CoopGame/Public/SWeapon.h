// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	void PlayerFireEffects(FVector TracerEnd);

	//Sub class of damage that returns the type of damage that is caused
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	//Sub class of Camera shake that allows the camera to shake when firing a weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ActualDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HeadShotDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ChestShotDamage;

	//Allows player to shoot
	//By tracing the world, from pawn eyes to cross hair location
	virtual void Fire();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	/*RPM Bullets per minutes fired*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	//Derived from rate of fire
	float TimeBetweenShots;

	/*Total Ammo Capacity the Rifle can hold*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int TotalAmmoCapacity;

	///*Ammo Capacity that can be Rifle can hold*/
	//UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	//int SingleAmmoMagCapacity;

	/*Ammo Capacity that can be Rifle can hold*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int AvailableAmmo;

	/*Ammo Capacity that can be Rifle can hold*/
	bool bCanReload;

public:

	virtual void StartFire();

	virtual void StopFire();

	virtual void ReloadWeapon();
};

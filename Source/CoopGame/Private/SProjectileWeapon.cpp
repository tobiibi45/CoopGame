// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

void ASProjectileWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector Eyelocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(Eyelocation, EyeRotation); // Returns the actors eyelocation and eye rotation

		FVector ShotDirection = EyeRotation.Vector();

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}
}

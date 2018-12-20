// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

void ASWeapon::Fire()
{

	AActor* MyOwner = GetOwner();
	if(MyOwner)
	{
		FVector Eyelocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(Eyelocation, EyeRotation); // Returns the actors eyelocation and eye rotation

		FVector ShotDirection = EyeRotation.Vector();


		//Sets the end of the line traced. Usually multiplied with a big number so you know it deffinitelly hits
		FVector TraceEnd = Eyelocation + (ShotDirection * 10000);

		//Quesrries that you can set to help improve the hit feedback
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner); // ignores th eowner weapon
		QueryParams.AddIgnoredActor(this); //ignores the mesh of the actor that fired
		QueryParams.bTraceComplex = true; //Enables means it tells you exactly where it has hit on a mesh

		//Particle Target Parameter
		FVector TracerEndPoint = TraceEnd;

		//Send the hit resuly
		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, Eyelocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			//Blocking hit process damage
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}

			TracerEndPoint = Hit.ImpactPoint;

		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), Eyelocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayerFireEffects();
	}

}

void ASWeapon::PlayerFireEffects()
{

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
}

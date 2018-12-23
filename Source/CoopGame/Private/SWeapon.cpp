// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

static int32 AmmoReserve = 1000;
static int32 SingleAmmoMagCapacity = 25;

// Sets default values
ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	ActualDamage = BaseDamage;
	HeadShotDamage = 5.0f;
	ChestShotDamage = 2.50f;

	RateOfFire = 600;
	LastFireTime = 0.0f;

	/*AmmoReserve = 1000;
	SingleAmmoMagCapacity = 25.0f;*/
	TotalAmmoCapacity = ((AmmoReserve - SingleAmmoMagCapacity) + SingleAmmoMagCapacity);
	AvailableAmmo = SingleAmmoMagCapacity;

	bCanReload = false;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.0f / RateOfFire;
} 


void ASWeapon::Fire()
{
	if (AvailableAmmo > 0)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			AvailableAmmo = AvailableAmmo - 1;
			if (AvailableAmmo < SingleAmmoMagCapacity)
			{
				bCanReload = true;
			}

			UE_LOG(LogTemp, Log, TEXT("Ammo: %s"), *FString::SanitizeFloat(AvailableAmmo));
			UE_LOG(LogTemp, Log, TEXT("Ammo Reserve: %s"), *FString::SanitizeFloat(TotalAmmoCapacity));

			FVector Eyelocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(Eyelocation, EyeRotation); // Returns the actors eye location and eye rotation

			FVector ShotDirection = EyeRotation.Vector();


			//Sets the end of the line traced. Usually multiplied with a big number so you know it definitely hits
			FVector TraceEnd = Eyelocation + (ShotDirection * 10000);

			//Queries that you can set to help improve the hit feedback
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner); // ignores th owner weapon
			QueryParams.AddIgnoredActor(this); //ignores the mesh of the actor that fired
			QueryParams.bTraceComplex = true; //Enables means it tells you exactly where it has hit on a mesh
			QueryParams.bReturnPhysicalMaterial = true; //Returns the physicalMaterial

			//Particle Target Parameter
			FVector TracerEndPoint = TraceEnd;

			//Send the hit result
			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, Eyelocation, TraceEnd, COLLISION_WEAPON_CHANNEL, QueryParams))
			{
				//Blocking hit process damage
				AActor* HitActor = Hit.GetActor();

				//Using weal pointers here helps to save memory (Hit.PhysMaterial.Get())
				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				ActualDamage = BaseDamage;
				switch (SurfaceType)
				{
				case SURFACE_FLESHVULNERABLE_HEAD:
					ActualDamage *= HeadShotDamage;
					break;
				case SURFACE_FLESHVULNERABLE_CHEST:
					ActualDamage *= ChestShotDamage;
					break;
				case SURFACE_FLESHDEFAULT:
					ActualDamage = BaseDamage;
					break;
				default:
					ActualDamage = BaseDamage;
					break;
				}

				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

				//Handles creating particle effects
				UParticleSystem* SelcetedEffect = nullptr;

				switch (SurfaceType)
				{
				case SURFACE_FLESHDEFAULT:
				case SURFACE_FLESHVULNERABLE_HEAD:
				case SURFACE_FLESHVULNERABLE_CHEST:
					//case SURFACE_FLESHCHEST:
					SelcetedEffect = FleshImpactEffect;
					break;
				default:
					SelcetedEffect = DefaultImpactEffect;
					break;
				}

				if (SelcetedEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelcetedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				}


				TracerEndPoint = Hit.ImpactPoint;

			}

			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), Eyelocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
			}

			PlayerFireEffects(TracerEndPoint);

			LastFireTime = GetWorld()->TimeSeconds;
		}
	}

}

void ASWeapon::StartFire()
{
	//Example, LastFiREtIME = 1, TimeBetweenShots = 0.1, TimeSeconds = 1. Means delay is 0.1.
	//Fmath::Max clamps th delay to 0 so it doesnt go below 0;
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::ReloadWeapon()
{
	if (AvailableAmmo == SingleAmmoMagCapacity || TotalAmmoCapacity == 0)
	{
		bCanReload = false;
		return;
	}

	if (bCanReload)
	{
		if (AvailableAmmo < SingleAmmoMagCapacity)
		{

			float BulletsToLoad = SingleAmmoMagCapacity - AvailableAmmo;

			if (TotalAmmoCapacity > BulletsToLoad)
			{
				TotalAmmoCapacity = TotalAmmoCapacity - BulletsToLoad;
				AvailableAmmo = AvailableAmmo + BulletsToLoad;
			}
			else
			{
				BulletsToLoad = TotalAmmoCapacity;
				AvailableAmmo = AvailableAmmo + BulletsToLoad;
			}
		}
		bCanReload = false;
	}
}

void ASWeapon::PlayerFireEffects(FVector TracerEnd)
{
	/*PLays the muzzle fire effect */
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	/*Plays the smoke trail effect when a gun is fired*/
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEnd);
		}
	}

	/*Makes the Camera shake when firing a weapon*/
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}


}

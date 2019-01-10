// Fill out your copyright notice in the Description page of Project Settings.

#include "Challenges/SExplosiveBarrel.h"
#include "CoopGame.h"
#include "Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	//Allows radial collison to affect all physics bodies;
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetCanEverAffectNavigation(false);
	RootComponent = MeshComp;


	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	//Bind our ON health changed function to the HealthComp
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = EXPLOSION_BARRELL_EXPLOSION_RADIUS;
	RadialForceComp->bImpulseVelChange = true; //More consistent application of the velocity
	RadialForceComp->bAutoActivate = false; //Prevents the barrel from ticking and only uses FireImpulse once. In other words, only explodes once
	RadialForceComp->bIgnoreOwningActor = true; //Ignores itself.. Only Apply force to other physics bodies;

	SPhereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SPhereComp->SetSphereRadius(EXPLOSION_BARRELL_EXPLOSION_RADIUS);
	SPhereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);  //Don't want physics on this
	SPhereComp->SetCollisionResponseToAllChannels(ECR_Ignore); //helps to reduce any unnecessary overlap
	SPhereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SPhereComp->SetupAttachment(RootComponent);

	ExplosionImpulse = EXPLOSION_BARRELL_FORCE;

	ExplosionDamage = EXPLOSION_BARRELL_EXPLOSION_DAMAGE;
	ExplosionRadius = EXPLOSION_BARRELL_EXPLOSION_RADIUS;

	bExploded = false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void ASExplosiveBarrel::OnHealthChanged(AActor* OwnigActor, USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}

	if (Health < EXPLOSION_BARRELL_HEALTH)
	{
		bExploded = true;
		OnRep_Exploded();

		//MeshComp->SetVisibility(false, true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (Role == ROLE_Authority)
		{
			//Boost the barrel upwards to simulate it exploding
			FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
			MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

			//Damage or Apply force to any physics body within range
			RadialForceComp->FireImpulse();

			TArray<AActor*> IgnoredActors;
			IgnoredActors.Add(this);

			//Apply damage
			UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

			//Delete Actor immediately
			SetLifeSpan(2.0f);
		}
	}
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	//Play Particle effect
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::SpawnSoundAttached(ExplodeSound, RootComponent);  //Attached is useed because it is getting played for several seconds and because the pawn moves and we are using 3D sound
	//Change Original Material of Barrel to exploded material
	MeshComp->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}

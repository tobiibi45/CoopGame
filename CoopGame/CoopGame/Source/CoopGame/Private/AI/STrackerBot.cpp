// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Math/UnrealMath.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SPhereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SPhereComp->SetSphereRadius(TRACKERBOT_EXPLOSION_RADIUS);
	SPhereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);  //Don't want physics on this
	SPhereComp->SetCollisionResponseToAllChannels(ECR_Ignore); //helps to reduce any unnecessary overlap
	SPhereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SPhereComp->SetupAttachment(RootComponent);

	MovementForce = TRACKER_BOT_MOVEMENT_FORCE;
	RequiredTargetToDistance = MIN_DISTANCE_TO_TARGET;

	bUseVelocityChange = false;

	ExplosionDamage = TRACKERBOT_EXPLOSION_DAMAGE;
	SelfDamageInterval = TRACKERBOT_SELF_DAMAGE_INTERVAL;
	ExplosionRadius = TRACKERBOT_EXPLOSION_RADIUS;

	bExploded = false;

	bStartedSelfDestructSequence = false;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		//Find th first path to move to
		NextPathPoint = GetNextPathPoint();

		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckTrackerBot, 1.0f, true);
	}
}

void ASTrackerBot::HandleTakeDamage(AActor* OwnigActor, USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	if (Health <= 0)
	{
		SelfDestruct();
	}

}

FVector ASTrackerBot::GetNextPathPoint()
{
	//Hack to get player location

	AActor*  PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if(NavPath && NavPath->PathPoints.Num() > 1)
	{
		//return the next point in the path array
		return NavPath->PathPoints[1];
	}

	//Failed to find a path
	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct()
{
	if(bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		//Increase damage based on the power level
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		//Apply damage
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		//Delete Actor immediately
		SetLifeSpan(2.0f);
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, TRACKERBOT_SELF_DAMAGE, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::OnCheckTrackerBot()
{

	//distance  to check for nearby bots
	const float Radius = TRACKERBOT_DISTANCE_TO_BOTS;

	//Create temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	//Only find pawns. (Players or AI bots)
	FCollisionObjectQueryParams QueryParams;

	//Our tracker bot mesh component is set to physics body in blueprint (default profile of physics simulated  bodies)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps; //An array that contains the overlap results
	if(GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape))
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}

	NumOfBots = 0;

	//Loop over the results using a range based for loop
	for (auto Result : Overlaps)
	{
		//Check if we overlapped with another tracker bot (ignoring players and other bots types)
		auto Bot = Cast<ASTrackerBot>(Result.GetActor());
		//Ignore this trackerbot instance
		if(Bot && Bot != this)
		{
			NumOfBots++;
			DrawDebugString(GetWorld(), GetActorLocation(), FString::FromInt(NumOfBots), this, FColor::Black, 1.0f, true);
		}
	}

	const int32 MaxPowerLevel = 4;

	//Clamp between min=0 and max = 4
	PowerLevel = FMath::Clamp(NumOfBots, 0, MaxPowerLevel);

	//Update the material Color
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		//Convert to a float between 0 and 1 just like an Alpha value of a texture. Now the material can be set up without having to know the max power level
		//Which can be tweaked many times by gameplay decision (would mean we need to keep 2 places up to date)
		float Alpha = PowerLevel / static_cast<float>(MaxPowerLevel);

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	DrawDebugString(GetWorld(), FVector(0,0,0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();


		if (DistanceToTarget <= RequiredTargetToDistance)
		{
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_Name, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 0, 1.0f);
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestructSequence && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		if (PlayerPawn)
		{
			//We overlapped a player

			if (Role == ROLE_Authority)
			{
				//Start self destruction sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestructSequence = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);  //Attached is useed because it is getting played for several seconds and because the pawn moves and we are using 3D sound

		}
	}
}

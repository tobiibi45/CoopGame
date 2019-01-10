// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "CoopGame.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{

	PowerupInterval = POWERUP_INTERVAL_DEFAULT;

	TotalNumOfTicks = POWERUP_TOTAL_NUMBER_OF_TICKS_DEFAULT;

	bIsPowerupActive = false;

	SetReplicates(true);
}


void ASPowerupActor::OnTickPowerup()
{
	TickProcessed++;

	OnPowerupTicked();

	if (TickProcessed >= TotalNumOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();
		//Delete Timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::ActivatePowerup(AActor* ActivateFor)
{
	OnActivated(ActivateFor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}
void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);

}


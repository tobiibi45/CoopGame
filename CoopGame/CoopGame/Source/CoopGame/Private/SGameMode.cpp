// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "CoopGame.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/PlayerController.h"

ASGameMode::ASGameMode()
{
	TimerBetweenWaves = HORDMODE_TIME_BETWEEN_ROUNDS_DEFAULT;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	CurrentGameType = GameModeType::DeathBased;

	bDeathModeActive = false;
	bIsHardModeActive = false;

}


void ASGameMode::StartWave()
{
	WaveCount++;

	if (bIsHardModeActive)
	{
		TimerBetweenWaves = HORDMODE_TIME_BETWEEN_ROUNDS_HARD;
		NrOfBotsToSpawn = HORDMODE_HARD_MULTIPLIER * WaveCount;
	}
	else
	{
		TimerBetweenWaves = HORDMODE_TIME_BETWEEN_ROUNDS_EASY;
		NrOfBotsToSpawn = HORDMODE_EASY_MULTIPLIER * WaveCount;
	}

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	switch (CurrentGameType)
	{
	case GameModeType::DeathBased:
		bDeathModeActive = true;
		break;
	case GameModeType::TimeBased:
		bDeathModeActive = false;
		PrepareForNextWave();
		break;
	default:
		PrepareForNextWave();
		break;
	}

}

void ASGameMode::PrepareForNextWave()
{

	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimerBetweenWaves, false);
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreaparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreaparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		PrepareForNextWave();
	}
	
	
}

void ASGameMode::CheckAnyPlayerIsAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (ensure(HealthComp && HealthComp->GetHealth() > 0.0f)) //Makes sure it breaks if this check fails
			{
				//A player is Alve
				return;
			}
		}
	}

	GameOver();
}

void ASGameMode::GameOver()
{
	EndWave();

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died"));
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDeathModeActive)
	{
		CheckWaveState();
	}

	CheckAnyPlayerIsAlive();
}

void ASGameMode::SpawnBotTimerElapsed()
{

	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

UENUM(BlueprintType)
enum  GameModeType
{
	TimeBased,
	DeathBased
};

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
protected:

	//Bots to spawn in current wave
	int32 NrOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimerBetweenWaves;
protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	//Start Spawning Bots
	void StartWave();

	//Stop Spawning Bots
	void EndWave();

	//Set timer for next wave
	void PrepareForNextWave();

	//Bot Spawner timer
	FTimerHandle TimerHandle_BotSpawner;

	//Next wave timer
	FTimerHandle TimerHandle_NextWaveStart;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	TEnumAsByte<GameModeType> CurrentGameType;

	void CheckWaveState();

	bool bDeathModeActive;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	bool bIsHardModeActive;

	void CheckAnyPlayerIsAlive();
	
	void GameOver();
public:
	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	ASGameMode();
};

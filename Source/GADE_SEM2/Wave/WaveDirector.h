#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveTypes.h"
#include "WaveDirector.generated.h"

class AEnemySpawner;
class UEnemyData;
class ATdGameState;

/**
 * Owns wave cadence. Spawners are dumb launchers on generated paths.
 * Fill ComputeBudget / ClassifyStyle / ComposeWave / PickPath so they
 * match the names in your 400-500 word document.
 */
UCLASS()
class GADE_SEM2_API AWaveDirector : public AActor
{
	GENERATED_BODY()

public:
	AWaveDirector();

	UPROPERTY(EditAnywhere, Category = "Td|Waves")
	float WaveInterval = 12.f;

	UPROPERTY(EditAnywhere, Category = "Td|Waves")
	float BaseBudget = 30.f;

	UPROPERTY(EditAnywhere, Category = "Td|Waves")
	float BudgetPerSecond = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Td|Waves")
	float PressureWindow = 20.f;

	UPROPERTY(EditAnywhere, Category = "Td|Roster")
	TArray<TObjectPtr<UEnemyData>> EnemyRoster;

	void StartMatch(const TArray<AEnemySpawner*>& InSpawners);
	void StopMatch();

	UFUNCTION(BlueprintPure, Category = "Td|Waves")
	int32 GetWaveIndex() const { return WaveIndex; }

	UFUNCTION(BlueprintPure, Category = "Td|Waves")
	float GetLastBudget() const { return LastBudget; }

	UFUNCTION(BlueprintPure, Category = "Td|Waves")
	ETdPlayStyle GetLastStyle() const { return LastStyle; }

protected:
	void CollectTelemetry(FTdPlayerTelemetry& Out) const;
	ETdPlayStyle ClassifyStyle(const FTdPlayerTelemetry& T) const;
	float ComputeBudget(const FTdPlayerTelemetry& T) const;
	void ComposeWave(float Budget, const FTdPlayerTelemetry& T, TArray<FTdWaveSpawn>& Out) const;
	int32 PickPath(const FTdPlayerTelemetry& T, ETdEnemyBehaviour ForType) const;
	void Dispatch(const TArray<FTdWaveSpawn>& Wave);

	void OnWaveTimer();

	TArray<TObjectPtr<AEnemySpawner>> Spawners;
	FTimerHandle WaveTimer;
	int32 WaveIndex = 0;
	float LastBudget = 0.f;
	ETdPlayStyle LastStyle = ETdPlayStyle::Unknown;
};

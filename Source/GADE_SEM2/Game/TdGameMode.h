#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TdGameMode.generated.h"

class AMapGenerator;
class AEnemySpawner;
class AEnemy;
class UDefenderData;
class UEnemyData;

UCLASS()
class GADE_SEM2_API ATdGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATdGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<AMapGenerator> GeneratorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<class ADefender> DefenderClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Data")
	TObjectPtr<UDefenderData> StarterDefender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Data")
	TObjectPtr<UEnemyData> StarterEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Data")
	TArray<TObjectPtr<UEnemyData>> EnemyRoster;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Data")
	TArray<TObjectPtr<UDefenderData>> DefenderRoster;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<class AWaveDirector> WaveDirectorClass;

	UFUNCTION(BlueprintCallable, Category = "Td")
	void SetSelectedDefenderIndex(int32 Index);

	UFUNCTION(BlueprintPure, Category = "Td")
	UDefenderData* GetSelectedDefenderData() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance")
	int32 StartingGold = 80;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance", meta = (ClampMin = "0.5"))
	float SpawnInterval = 2.5f;

	// ---- difficulty curve ----
	// Lecturer feedback: defenders out-scale enemies and pressure never builds.
	// Every wave multiplies enemy stats and tightens the spawn cadence.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "5.0"))
	float WaveLength = 20.f;

	/** +18% enemy HP per wave, compounding. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "0.0"))
	float EnemyHealthPerWave = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "0.0"))
	float EnemyDamagePerWave = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "0.0"))
	float EnemySpeedPerWave = 0.03f;

	/** Speed is capped so enemies never outrun the waypoint acceptance radius. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "1.0"))
	float MaxEnemySpeedScale = 1.6f;

	/** Spawn interval is multiplied by this each wave. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SpawnIntervalPerWave = 0.88f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Waves", meta = (ClampMin = "0.25"))
	float MinSpawnInterval = 1.6f;

	/** Each defender bought raises the price of the next one, so spam is not free. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0"))
	int32 DefenderCostStep = 15;

	/** Paid out when a wave rolls over, giving the player a regular decision point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0"))
	int32 GoldPerWave = 25;

	UFUNCTION(BlueprintPure, Category = "Td|Balance")
	int32 GetNextDefenderCost() const;

	/** Slots call this after a successful purchase. */
	void NotifyDefenderPurchased();

	/** GameState wave clock calls this. Scales enemies and tightens spawns. */
	void NotifyWaveElapsed();

	UFUNCTION(BlueprintCallable, Category = "Td")
	void StartNewMatch();

	UFUNCTION(BlueprintCallable, Category = "Td")
	void RestartMatch();

	UFUNCTION(BlueprintCallable, Category = "Td")
	void SetPausedMatch(bool bPause);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void NotifyTowerDestroyed();

protected:
	virtual void BeginPlay() override;

	void FramePlayerOnTower();

	UPROPERTY()
	TObjectPtr<AMapGenerator> Generator;

	UPROPERTY()
	TObjectPtr<class AWaveDirector> WaveDirector;

	UPROPERTY()
	TArray<TObjectPtr<AEnemySpawner>> ActiveSpawners;

	float CurrentSpawnInterval = 2.5f;
};

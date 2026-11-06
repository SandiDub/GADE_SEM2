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

	/**
	 * Price step applied per defender TYPE. Buying a third Gunner raises the
	 * Gunner price only, so diversifying is the cheap play.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0"))
	int32 DefenderCostStep = 15;

	/** Paid out when a wave rolls over, giving the player a regular decision point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0"))
	int32 GoldPerWave = 25;

	/**
	 * Added to the kill-gold multiplier each wave. 0.35 puts wave 5 at roughly
	 * 2.4x wave 1, which tracks the +18%/wave enemy health curve.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0.0"))
	float GoldGrowthPerWave = 0.35f;

	/** Ceiling so a long match cannot turn gold into a non-resource. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "1.0"))
	float MaxGoldMultiplier = 4.f;

	// ---- periodic repair ----

	/** Every N defenders placed, the generator releases more build slots. 0 disables. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0"))
	int32 DefendersPerSlotUnlock = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Economy", meta = (ClampMin = "0"))
	int32 SlotsPerUnlock = 3;

	/** Heal everything on the player's side every N waves. 0 disables it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Repair", meta = (ClampMin = "0"))
	int32 RepairWaveInterval = 3;

	/** Fraction of max health restored. 0.2 = 20%. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance|Repair", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RepairPercent = 0.2f;

	/** Cost of a specific type: its data asset Cost plus this player's own step for that type. */
	UFUNCTION(BlueprintPure, Category = "Td|Balance")
	int32 GetDefenderCost(UDefenderData* Data) const;

	/** Cost of whatever is currently selected. What the HUD shows. */
	UFUNCTION(BlueprintPure, Category = "Td|Balance")
	int32 GetNextDefenderCost() const;

	/** Slots call this after a successful purchase. */
	void NotifyDefenderPurchased(UDefenderData* Data);

	/** GameState wave clock calls this. Scales enemies and tightens spawns. */
	void NotifyWaveElapsed();

	/** Restores RepairPercent of max health to the tower and every living defender. */
	UFUNCTION(BlueprintCallable, Category = "Td|Balance|Repair")
	void ApplyWaveRepair();

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

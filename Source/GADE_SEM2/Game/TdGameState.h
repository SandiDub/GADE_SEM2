#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TdTypes.h"
#include "TdGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTdWaveChanged, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTdGoldChanged, int32, Gold, int32, NextCost);

UCLASS()
class GADE_SEM2_API ATdGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ATdGameState();

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Seed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Gold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Leaks = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 GoldSpent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 SelectedDefenderIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	float Pressure = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	ETdMatchState MatchState = ETdMatchState::Waiting;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	float MatchTime = 0.f;

	// ---- wave scaling: the lecturer asked for pressure that grows across waves ----

	UPROPERTY(BlueprintReadOnly, Category = "Td|Waves")
	int32 WaveNumber = 0;

	/** Seconds remaining until the next wave increments. Drives the HUD bar. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|Waves")
	float WaveCountdown = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td|Waves")
	float WaveLength = 20.f;

	/** Compounding multipliers applied to every enemy at spawn time. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|Waves")
	float EnemyHealthScale = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td|Waves")
	float EnemyDamageScale = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td|Waves")
	float EnemySpeedScale = 1.f;

	/** How many defenders the player has bought. Drives cost escalation. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|Economy")
	int32 DefendersPurchased = 0;

	UPROPERTY(BlueprintAssignable, Category = "Td|Events")
	FOnTdWaveChanged OnWaveChanged;

	UPROPERTY(BlueprintAssignable, Category = "Td|Events")
	FOnTdGoldChanged OnGoldChanged;

	UFUNCTION(BlueprintCallable, Category = "Td")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Td")
	bool TrySpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void AddKill() { ++Kills; }

	UFUNCTION(BlueprintCallable, Category = "Td")
	void RegisterLeak() { ++Leaks; }

	UFUNCTION(BlueprintCallable, Category = "Td")
	void RegisterSpend(int32 Amount) { if (Amount > 0) { GoldSpent += Amount; } }

	UFUNCTION(BlueprintCallable, Category = "Td")
	void AddPressure(float Amount);

	UFUNCTION(BlueprintPure, Category = "Td")
	float GetPressure(float Window) const { return Pressure; }

	UFUNCTION(BlueprintPure, Category = "Td")
	bool CanAfford(int32 Amount) const { return Gold >= Amount; }

	/** Called by the GameMode each time the wave clock rolls over. */
	void AdvanceWave(float HealthPerWave, float DamagePerWave, float SpeedPerWave, float MaxSpeedScale);

	void ResetForNewMatch(int32 InSeed, int32 StartingGold, float InWaveLength);

	/** One call that feeds an entire UMG widget. */
	UFUNCTION(BlueprintPure, Category = "Td|HUD")
	FTdHudSnapshot GetHudSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Td|HUD")
	FText GetMatchTimeText() const;

	virtual void Tick(float DeltaSeconds) override;
};

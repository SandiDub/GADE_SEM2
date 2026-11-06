#pragma once

#include "CoreMinimal.h"
#include "WaveTypes.generated.h"

class UEnemyData;

UENUM(BlueprintType)
enum class ETdEnemyBehaviour : uint8
{
	Grunt   UMETA(DisplayName = "Grunt — aggro defenders, then resume path"),
	Runner  UMETA(DisplayName = "Runner — ignore defenders, rush tower"),
	Bruiser UMETA(DisplayName = "Bruiser — prefer peeling defenders")
};

/**
 * Rolled per spawn by the procedural mutation pass. Each one is a trade, not
 * a straight buff, so a mutated unit is a different problem rather than a
 * harder one and the wave director's threat budget stays honest.
 */
UENUM(BlueprintType)
enum class ETdEnemyMutation : uint8
{
	None      UMETA(DisplayName = "Baseline"),
	Armoured  UMETA(DisplayName = "Armoured — bulkier and slower"),
	Swift     UMETA(DisplayName = "Swift — faster and frailer"),
	Volatile  UMETA(DisplayName = "Volatile — hits harder, dies quicker"),
	Giant     UMETA(DisplayName = "Giant — large, tanky, sluggish"),
	Runt      UMETA(DisplayName = "Runt — small, quick, fragile")
};

UENUM(BlueprintType)
enum class ETdDefenderBehaviour : uint8
{
	Gunner UMETA(DisplayName = "Gunner — single target"),
	Frost  UMETA(DisplayName = "Frost — slow on hit"),
	Mortar UMETA(DisplayName = "Mortar — splash")
};

UENUM(BlueprintType)
enum class ETdPlayStyle : uint8
{
	Unknown    UMETA(DisplayName = "Unknown"),
	OneLane    UMETA(DisplayName = "One-lane turtle"),
	CheapSpam  UMETA(DisplayName = "Cheap spam"),
	MortarNest UMETA(DisplayName = "Mortar turtle"),
	SpreadThin UMETA(DisplayName = "Spread thin")
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdLaneCoverage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PathIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DefenderCount = 0;
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdPlayerTelemetry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float TowerHealthNorm = 1.f;

	/** HP fraction lost over the last PressureWindow seconds. 0 = calm, 1 = melted. */
	UPROPERTY(BlueprintReadOnly)
	float Pressure = 0.f;

	UPROPERTY(BlueprintReadOnly)
	int32 Leaks = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Gold = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 GoldSpent = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Gunners = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Frosts = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Mortars = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTdLaneCoverage> Lanes;

	UPROPERTY(BlueprintReadOnly)
	ETdPlayStyle Style = ETdPlayStyle::Unknown;
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdWaveSpawn
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UEnemyData> Enemy;

	UPROPERTY()
	int32 PathIndex = 0;
};

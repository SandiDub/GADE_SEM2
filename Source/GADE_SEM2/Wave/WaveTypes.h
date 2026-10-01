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

#pragma once

#include "CoreMinimal.h"
#include "TdTypes.generated.h"

/** Gameplay classification of one heightfield cell. Tags are the source of truth, not mesh colour. */
UENUM(BlueprintType)
enum class ETdCellTag : uint8
{
	Unset      UMETA(DisplayName = "Unset"),
	Path       UMETA(DisplayName = "Path"),
	Buildable  UMETA(DisplayName = "Buildable"),
	Blocked    UMETA(DisplayName = "Blocked"),
	TowerPad   UMETA(DisplayName = "Tower Pad")
};

UENUM(BlueprintType)
enum class ETdTeam : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Player  UMETA(DisplayName = "Player"),
	Enemy   UMETA(DisplayName = "Enemy")
};

UENUM(BlueprintType)
enum class ETdMatchState : uint8
{
	Waiting  UMETA(DisplayName = "Waiting"),
	Playing  UMETA(DisplayName = "Playing"),
	Paused   UMETA(DisplayName = "Paused"),
	GameOver UMETA(DisplayName = "Game Over")
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdGridCoord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 X = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Y = 0;

	FTdGridCoord() = default;
	FTdGridCoord(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FTdGridCoord& Other) const { return X == Other.X && Y == Other.Y; }
	friend uint32 GetTypeHash(const FTdGridCoord& C) { return HashCombine(GetTypeHash(C.X), GetTypeHash(C.Y)); }
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdPath
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FTdGridCoord> Cells;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> Waypoints;
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdSlotData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FTdGridCoord Cell;

	UPROPERTY(BlueprintReadWrite)
	FTransform WorldTransform = FTransform::Identity;

	/** World Z of the slot. Feed this into defender range for the complexity mark. */
	UPROPERTY(BlueprintReadWrite)
	float HeightAdvantage = 0.f;

	/** Which generated path this slot guards. Drives the per-lane quota and HUD coverage. */
	UPROPERTY(BlueprintReadWrite)
	int32 PathIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct GADE_SEM2_API FGeneratedMap
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Seed = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Resolution = 0;

	UPROPERTY(BlueprintReadOnly)
	float CellSize = 100.f;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> Height;

	UPROPERTY(BlueprintReadOnly)
	TArray<ETdCellTag> Tags;

	UPROPERTY(BlueprintReadOnly)
	FTdGridCoord TowerCell;

	UPROPERTY(BlueprintReadOnly)
	FTransform TowerTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTdPath> Paths;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTdSlotData> Slots;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTransform> SpawnTransforms;
};

/**
 * Everything a UMG widget needs, in one BlueprintPure call.
 * Bind widget fields to ATdGameState::GetHudSnapshot() instead of
 * reaching into GameMode/actors from Blueprint.
 */
USTRUCT(BlueprintType)
struct GADE_SEM2_API FTdHudSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 Gold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 NextDefenderCost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	bool bCanAffordNext = false;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	FText SelectedDefenderName;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 WaveNumber = 0;

	/** 0..1 progress to the next wave. Drive a progress bar with this. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	float WaveProgress = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	float TowerHealthPercent = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 DefendersAlive = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 SlotsTotal = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 EnemiesAlive = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 Leaks = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	float MatchTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	int32 Seed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	ETdMatchState MatchState = ETdMatchState::Waiting;

	/** Defenders currently guarding each generated path, indexed by path. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	TArray<int32> LaneDefenders;

	/** Slots that exist on each generated path, indexed by path. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|HUD")
	TArray<int32> LaneSlots;
};

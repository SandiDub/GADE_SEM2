#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TdTypes.h"
#include "MapGenerator.generated.h"

class ACentralTower;
class ADefenderSlot;
class AEnemySpawner;
class UMaterialInterface;

/**
 * Owns the generation pipeline and the runtime terrain mesh.
 * GameMode calls GenerateWorld(Seed). This actor does not Tick combat.
 *
 * Pipeline (you fill each step):
 *   Seed -> heightfield -> tower cell -> 3+ spawns
 *        -> A* paths -> carve/tag -> slots -> mesh -> child actors
 */
UCLASS()
class GADE_SEM2_API AMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMapGenerator();

	UPROPERTY(VisibleAnywhere, Category = "Td|Terrain")
	TObjectPtr<UProceduralMeshComponent> TerrainMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain", meta = (ClampMin = "16", ClampMax = "256"))
	int32 Resolution = 96;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain", meta = (ClampMin = "10.0"))
	float CellSize = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain", meta = (ClampMin = "3"))
	int32 PathCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain", meta = (ClampMin = "1"))
	int32 MaxSlots = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	float HeightScale = 280.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	float BasinStrength = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	float RoadDepth = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	float SlopeBuildableMax = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain", meta = (ClampMin = "1"))
	int32 SlotSpacing = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	float WaypointLift = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain")
	TObjectPtr<UMaterialInterface> TerrainMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<ACentralTower> TowerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<ADefenderSlot> SlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<AEnemySpawner> SpawnerClass;

	UFUNCTION(BlueprintCallable, Category = "Td|Terrain")
	const FGeneratedMap& GetMap() const { return Map; }

	/** Destroys previous children, runs the pipeline, builds the mesh, spawns gameplay actors. */
	UFUNCTION(BlueprintCallable, Category = "Td|Terrain")
	void GenerateWorld(int32 Seed);

protected:
	void ClearPreviousGeneration();
	void SpawnGameplayActors();
	void DrawDebugOverlay() const;

	
	void BuildHeightfield(FRandomStream& Rng);
	void ChooseTowerCell();
	void ChooseSpawnCells(FRandomStream& Rng);
	void BuildPaths();
	bool FindPathAStar(const FTdGridCoord& Start, const FTdGridCoord& Goal, FTdPath& OutPath) const;
	void CarvePaths();
	void ExtractSlots();
	void BuildMesh();

	float StepCost(const FTdGridCoord& From, const FTdGridCoord& To) const;
	float Heuristic(const FTdGridCoord& From, const FTdGridCoord& To) const;

	float GetHeight(const FTdGridCoord& C) const;
	FVector GetCellWorld(const FTdGridCoord& C, float ExtraZ = 0.f) const;
	void RefreshWorldAnchors();
	ETdCellTag GetTag(const FTdGridCoord& C) const;
	void SetTag(const FTdGridCoord& C, ETdCellTag Tag);

	UPROPERTY()
	FGeneratedMap Map;

	TArray<FTdGridCoord> SpawnCells;

	UPROPERTY()
	TObjectPtr<ACentralTower> SpawnedTower;

	UPROPERTY()
	TArray<TObjectPtr<ADefenderSlot>> SpawnedSlots;

	UPROPERTY()
	TArray<TObjectPtr<AEnemySpawner>> SpawnedSpawners;
};

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
class UStaticMesh;
class UInstancedStaticMeshComponent;

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

	/** Each generated path is guaranteed this many slots before the cap is shared out. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Terrain", meta = (ClampMin = "1"))
	int32 MinSlotsPerPath = 3;

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

	// ---- terrain palette ----
	// Baked into the mesh as vertex colours, so the material can stay unlit
	// and the map reads identically regardless of scene lighting. Edit these
	// and press the regenerate button rather than fighting the sun.

	/** Enemy lanes. Should be the most legible colour on the map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Palette")
	FLinearColor PathColor = FLinearColor(0.45f, 0.10f, 0.85f);

	/** Ground the player may build on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Palette")
	FLinearColor BuildableColor = FLinearColor(0.07f, 0.16f, 0.30f);

	/** The pad under the central tower. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Palette")
	FLinearColor TowerPadColor = FLinearColor(0.f, 0.90f, 1.f);

	/** Unreachable terrain. Keep this darkest so the playable area pops. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Palette")
	FLinearColor BlockedColor = FLinearColor(0.04f, 0.05f, 0.11f);

	// ---- procedural prop scatter ----
	// Custom PCG pass. Runs on the heightfield this actor just generated and
	// uses the same seed, so the decoration is part of the generated world
	// rather than hand-placed dressing. Instanced meshes, so it is one draw
	// call per prop type no matter how many are placed.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Scatter")
	bool bScatterProps = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Scatter")
	TArray<TObjectPtr<UStaticMesh>> PropMeshes;

	/** Chance per eligible cell. 0.06 is sparse; above 0.2 the map gets busy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Scatter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PropDensity = 0.06f;

	/** Cells this far from a path or slot are left clear, so props never block play. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Scatter", meta = (ClampMin = "0"))
	int32 PropClearance = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Scatter")
	FVector2D PropScaleRange = FVector2D(0.6f, 1.8f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Scatter")
	TObjectPtr<UMaterialInterface> PropMaterial;

	// ---- unlockable slots ----

	/** Candidates held back at generation, released as the player builds. */
	UFUNCTION(BlueprintCallable, Category = "Td|Terrain")
	int32 UnlockSlots(int32 Count);

	UFUNCTION(BlueprintPure, Category = "Td|Terrain")
	int32 GetReservedSlotCount() const { return ReservedSlots.Num(); }

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
	void ScatterProps();
	bool IsCellClearForProp(const FTdGridCoord& C) const;

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

	/** Valid slot positions withheld at generation so the map can grow. */
	UPROPERTY()
	TArray<FTdSlotData> ReservedSlots;

	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> PropLayers;
};

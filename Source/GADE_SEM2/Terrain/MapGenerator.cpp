#include "MapGenerator.h"
#include "GridMath.h"
#include "Actors/CentralTower.h"
#include "Actors/DefenderSlot.h"
#include "Actors/EnemySpawner.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Algo/Reverse.h"

AMapGenerator::AMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	SetRootComponent(TerrainMesh);
	TerrainMesh->bUseAsyncCooking = false;
	TerrainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

void AMapGenerator::GenerateWorld(int32 Seed)
{
	ClearPreviousGeneration();

	Map = FGeneratedMap();
	Map.Seed = Seed;
	Map.Resolution = Resolution;
	Map.CellSize = CellSize;
	Map.Height.SetNumZeroed(Resolution * Resolution);
	Map.Tags.SetNumZeroed(Resolution * Resolution);

	FRandomStream Rng(Seed);

	BuildHeightfield(Rng);
	ChooseTowerCell();
	ChooseSpawnCells(Rng);
	BuildPaths();
	CarvePaths();
	RefreshWorldAnchors();
	ExtractSlots();
	BuildMesh();
	SpawnGameplayActors();

	if (bDrawDebug)
	{
		DrawDebugOverlay();
	}

	UE_LOG(LogTemp, Log, TEXT("Td: generated map seed %d  paths %d  slots %d"),
		Map.Seed, Map.Paths.Num(), Map.Slots.Num());
}

void AMapGenerator::ClearPreviousGeneration()
{
	if (SpawnedTower)
	{
		SpawnedTower->Destroy();
		SpawnedTower = nullptr;
	}
	for (ADefenderSlot* Slot : SpawnedSlots)
	{
		if (Slot) { Slot->Destroy(); }
	}
	SpawnedSlots.Reset();
	for (AEnemySpawner* Spawner : SpawnedSpawners)
	{
		if (Spawner) { Spawner->Destroy(); }
	}
	SpawnedSpawners.Reset();
	SpawnCells.Reset();

	if (TerrainMesh)
	{
		TerrainMesh->ClearAllMeshSections();
	}
}

void AMapGenerator::SpawnGameplayActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (TowerClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		SpawnedTower = World->SpawnActor<ACentralTower>(TowerClass, Map.TowerTransform, Params);
	}

	if (SlotClass)
	{
		for (const FTdSlotData& SlotData : Map.Slots)
		{
			ADefenderSlot* Slot = World->SpawnActor<ADefenderSlot>(SlotClass, SlotData.WorldTransform);
			if (Slot)
			{
				Slot->Configure(SlotData);
				SpawnedSlots.Add(Slot);
			}
		}
	}

	if (SpawnerClass)
	{
		for (int32 i = 0; i < Map.Paths.Num(); ++i)
		{
			const FTransform& Xform = Map.SpawnTransforms.IsValidIndex(i)
				? Map.SpawnTransforms[i]
				: FTransform::Identity;

			AEnemySpawner* Spawner = World->SpawnActor<AEnemySpawner>(SpawnerClass, Xform);
			if (Spawner)
			{
				Spawner->Configure(Map.Paths[i]);
				SpawnedSpawners.Add(Spawner);
			}
		}
	}
}

void AMapGenerator::DrawDebugOverlay() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Life = 20.f;

	for (const FTdPath& Path : Map.Paths)
	{
		for (int32 i = 1; i < Path.Waypoints.Num(); ++i)
		{
			DrawDebugLine(World, Path.Waypoints[i - 1], Path.Waypoints[i], FColor::Cyan, false, Life, 0, 6.f);
		}
	}

	for (const FTdSlotData& Slot : Map.Slots)
	{
		DrawDebugSphere(World, Slot.WorldTransform.GetLocation() + FVector(0, 0, 40.f), 30.f, 8, FColor::Yellow, false, Life, 0, 2.f);
	}

	DrawDebugBox(World, Map.TowerTransform.GetLocation() + FVector(0, 0, 80.f), FVector(60.f), FColor::White, false, Life, 0, 3.f);
}

// =============================================================================
// Helpers
// =============================================================================

namespace
{
	/** Deterministic [0,1] hash. Same seed always yields the same field. */
	float TdHash01(int32 X, int32 Y, int32 Seed, int32 Octave)
	{
		uint32 H = GetTypeHash(X);
		H = HashCombine(H, GetTypeHash(Y));
		H = HashCombine(H, GetTypeHash(Seed));
		H = HashCombine(H, GetTypeHash(Octave));
		H ^= H >> 16;
		H *= 0x7feb352dU;
		H ^= H >> 15;
		return (H & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
	}

	float TdValueNoise(float X, float Y, int32 Seed, int32 Octave)
	{
		const int32 X0 = FMath::FloorToInt(X);
		const int32 Y0 = FMath::FloorToInt(Y);
		const float Tx = FMath::SmoothStep(0.f, 1.f, FMath::Frac(X));
		const float Ty = FMath::SmoothStep(0.f, 1.f, FMath::Frac(Y));
		const float V00 = TdHash01(X0, Y0, Seed, Octave);
		const float V10 = TdHash01(X0 + 1, Y0, Seed, Octave);
		const float V01 = TdHash01(X0, Y0 + 1, Seed, Octave);
		const float V11 = TdHash01(X0 + 1, Y0 + 1, Seed, Octave);
		const float A = FMath::Lerp(V00, V10, Tx);
		const float B = FMath::Lerp(V01, V11, Tx);
		return FMath::Lerp(A, B, Ty);
	}

	float TdFbm(float X, float Y, int32 Seed)
	{
		// Layered value noise (Perlin, 1985: octaves, frequency doubles, amplitude halves).
		float Sum = 0.f;
		float Amp = 1.f;
		float Freq = 0.035f;
		float Norm = 0.f;
		for (int32 Octave = 0; Octave < 4; ++Octave)
		{
			Sum += TdValueNoise(X * Freq, Y * Freq, Seed, Octave) * Amp;
			Norm += Amp;
			Amp *= 0.5f;
			Freq *= 2.f;
		}
		return (Norm > 0.f) ? (Sum / Norm) : 0.f;
	}
}

float AMapGenerator::GetHeight(const FTdGridCoord& C) const
{
	const int32 I = TdGrid::Index(C, Resolution);
	return Map.Height.IsValidIndex(I) ? Map.Height[I] : 0.f;
}

ETdCellTag AMapGenerator::GetTag(const FTdGridCoord& C) const
{
	const int32 I = TdGrid::Index(C, Resolution);
	return Map.Tags.IsValidIndex(I) ? Map.Tags[I] : ETdCellTag::Unset;
}

void AMapGenerator::SetTag(const FTdGridCoord& C, ETdCellTag Tag)
{
	const int32 I = TdGrid::Index(C, Resolution);
	if (Map.Tags.IsValidIndex(I))
	{
		Map.Tags[I] = Tag;
	}
}

FVector AMapGenerator::GetCellWorld(const FTdGridCoord& C, float ExtraZ) const
{
	return TdGrid::CellCentre(C, CellSize, GetHeight(C) + ExtraZ) + GetActorLocation();
}

void AMapGenerator::RefreshWorldAnchors()
{
	for (FTdPath& Path : Map.Paths)
	{
		Path.Waypoints.Reset();
		Path.Waypoints.Reserve(Path.Cells.Num());
		for (const FTdGridCoord& C : Path.Cells)
		{
			Path.Waypoints.Add(GetCellWorld(C, WaypointLift));
		}
	}

	Map.SpawnTransforms.Reset();
	for (const FTdPath& Path : Map.Paths)
	{
		if (Path.Waypoints.Num() > 0)
		{
			Map.SpawnTransforms.Add(FTransform(Path.Waypoints[0]));
		}
	}

	Map.TowerTransform = FTransform(GetCellWorld(Map.TowerCell));
}

void AMapGenerator::BuildHeightfield(FRandomStream& Rng)
{
	(void)Rng;

	const float Cx = (Resolution - 1) * 0.5f;
	const float Cy = (Resolution - 1) * 0.5f;
	const float MaxDist = FMath::Sqrt(Cx * Cx + Cy * Cy);

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const float Noise = TdFbm(static_cast<float>(X), static_cast<float>(Y), Map.Seed);
			const float Dist = FVector2D(X - Cx, Y - Cy).Size();
			const float Basin = 1.f - FMath::Clamp(Dist / MaxDist, 0.f, 1.f);
			const float Wander = TdHash01(X, Y, Map.Seed, 99) * 18.f;
			Map.Height[TdGrid::Index(X, Y, Resolution)] =
				Noise * HeightScale - Basin * BasinStrength + Wander;
		}
	}
}

void AMapGenerator::ChooseTowerCell()
{
	const float Cx = (Resolution - 1) * 0.5f;
	const float Cy = (Resolution - 1) * 0.5f;
	const float InnerR = FMath::Max(3.f, Resolution * 0.2f);

	Map.TowerCell = FTdGridCoord(Resolution / 2, Resolution / 2);
	float BestH = TNumericLimits<float>::Max();

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const float Dist = FVector2D(X - Cx, Y - Cy).Size();
			if (Dist > InnerR)
			{
				continue;
			}
			const FTdGridCoord C(X, Y);
			const float H = GetHeight(C);
			if (H < BestH)
			{
				BestH = H;
				Map.TowerCell = C;
			}
		}
	}

	Map.TowerTransform = FTransform(GetCellWorld(Map.TowerCell));
}

void AMapGenerator::ChooseSpawnCells(FRandomStream& Rng)
{
	SpawnCells.Reset();

	const int32 Count = FMath::Max(3, PathCount);
	const float Cx = (Resolution - 1) * 0.5f;
	const float Cy = (Resolution - 1) * 0.5f;
	const float Radius = Resolution * 0.42f;
	const int32 MinSep = FMath::Max(6, Resolution / (Count * 2));
	const float BaseAngle = Rng.FRand() * 2.f * PI;

	const auto Accept = [&](const FTdGridCoord& Candidate) -> bool
	{
		if (!TdGrid::IsInside(Candidate, Resolution))
		{
			return false;
		}
		if (FMath::Abs(Candidate.X - Map.TowerCell.X) + FMath::Abs(Candidate.Y - Map.TowerCell.Y) < 8)
		{
			return false;
		}
		for (const FTdGridCoord& Existing : SpawnCells)
		{
			const int32 Dx = FMath::Abs(Existing.X - Candidate.X);
			const int32 Dy = FMath::Abs(Existing.Y - Candidate.Y);
			if (Dx <= MinSep && Dy <= MinSep)
			{
				return false;
			}
		}
		return true;
	};

	for (int32 i = 0; i < Count; ++i)
	{
		bool bPlaced = false;
		for (int32 Attempt = 0; Attempt < 24 && !bPlaced; ++Attempt)
		{
			const float Jitter = (Rng.FRand() - 0.5f) * (2.f * PI / Count) * 0.35f;
			const float Angle = BaseAngle + (2.f * PI * i) / Count + Jitter;
			const int32 X = FMath::Clamp(FMath::RoundToInt(Cx + FMath::Cos(Angle) * Radius), 2, Resolution - 3);
			const int32 Y = FMath::Clamp(FMath::RoundToInt(Cy + FMath::Sin(Angle) * Radius), 2, Resolution - 3);
			const FTdGridCoord Candidate(X, Y);
			if (Accept(Candidate))
			{
				SpawnCells.Add(Candidate);
				bPlaced = true;
			}
		}

		if (!bPlaced)
		{
			const float Angle = BaseAngle + (2.f * PI * i) / Count;
			SpawnCells.Add(FTdGridCoord(
				FMath::Clamp(FMath::RoundToInt(Cx + FMath::Cos(Angle) * Radius), 2, Resolution - 3),
				FMath::Clamp(FMath::RoundToInt(Cy + FMath::Sin(Angle) * Radius), 2, Resolution - 3)));
		}
	}
}

void AMapGenerator::BuildPaths()
{
	Map.Paths.Reset();
	Map.SpawnTransforms.Reset();

	for (const FTdGridCoord& Start : SpawnCells)
	{
		FTdPath Path;
		if (FindPathAStar(Start, Map.TowerCell, Path) && Path.Cells.Num() > 1)
		{
			Map.Paths.Add(Path);
			const FVector StartWorld = Path.Waypoints.Num() > 0 ? Path.Waypoints[0] : GetCellWorld(Start, WaypointLift);
			Map.SpawnTransforms.Add(FTransform(StartWorld));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Td: A* failed from (%d,%d) to tower. Relax height costs."), Start.X, Start.Y);
		}
	}
}

bool AMapGenerator::FindPathAStar(const FTdGridCoord& Start, const FTdGridCoord& Goal, FTdPath& OutPath) const
{
	// A* (Hart, Nilsson, Raphael 1968). 4-connected grid, Manhattan heuristic.
	OutPath = FTdPath();
	if (!TdGrid::IsInside(Start, Resolution) || !TdGrid::IsInside(Goal, Resolution))
	{
		return false;
	}

	TArray<FTdGridCoord> Open;
	TSet<FTdGridCoord> Closed;
	TMap<FTdGridCoord, FTdGridCoord> Parent;
	TMap<FTdGridCoord, float> GScore;

	Open.Add(Start);
	GScore.Add(Start, 0.f);

	TArray<FTdGridCoord> Neighbours;
	Neighbours.Reserve(4);

	while (Open.Num() > 0)
	{
		int32 BestIdx = 0;
		float BestF = GScore[Open[0]] + Heuristic(Open[0], Goal);
		for (int32 i = 1; i < Open.Num(); ++i)
		{
			const float F = GScore[Open[i]] + Heuristic(Open[i], Goal);
			if (F < BestF)
			{
				BestF = F;
				BestIdx = i;
			}
		}

		const FTdGridCoord Current = Open[BestIdx];
		Open.RemoveAtSwap(BestIdx);

		if (Current == Goal)
		{
			TArray<FTdGridCoord> Rev;
			FTdGridCoord Walk = Goal;
			Rev.Add(Walk);
			while (!(Walk == Start))
			{
				const FTdGridCoord* P = Parent.Find(Walk);
				if (!P)
				{
					return false;
				}
				Walk = *P;
				Rev.Add(Walk);
			}
			Algo::Reverse(Rev);
			OutPath.Cells = MoveTemp(Rev);
			OutPath.Waypoints.Reserve(OutPath.Cells.Num());
			for (const FTdGridCoord& C : OutPath.Cells)
			{
				OutPath.Waypoints.Add(GetCellWorld(C, WaypointLift));
			}
			return true;
		}

		Closed.Add(Current);
		TdGrid::Neighbours4(Current, Resolution, Neighbours);
		for (const FTdGridCoord& N : Neighbours)
		{
			if (Closed.Contains(N))
			{
				continue;
			}

			const float TentativeG = GScore[Current] + StepCost(Current, N);
			const float* ExistingG = GScore.Find(N);
			if (ExistingG && TentativeG >= *ExistingG)
			{
				continue;
			}

			Parent.Add(N, Current);
			GScore.Add(N, TentativeG);
			Open.AddUnique(N);
		}
	}

	return false;
}

float AMapGenerator::StepCost(const FTdGridCoord& From, const FTdGridCoord& To) const
{
	const float Dh = FMath::Abs(GetHeight(From) - GetHeight(To));
	const float Wander = TdHash01(To.X, To.Y, Map.Seed, 7) * 0.35f;
	return 1.f + 0.045f * Dh + Wander;
}

float AMapGenerator::Heuristic(const FTdGridCoord& From, const FTdGridCoord& To) const
{
	return static_cast<float>(FMath::Abs(From.X - To.X) + FMath::Abs(From.Y - To.Y));
}

void AMapGenerator::CarvePaths()
{
	auto LowerRoad = [this](const FTdGridCoord& C)
	{
		const int32 I = TdGrid::Index(C, Resolution);
		if (Map.Height.IsValidIndex(I))
		{
			Map.Height[I] -= RoadDepth;
		}
		SetTag(C, ETdCellTag::Path);
	};

	for (const FTdPath& Path : Map.Paths)
	{
		for (const FTdGridCoord& C : Path.Cells)
		{
			LowerRoad(C);
		}
	}

	TArray<FTdGridCoord> Neighbours;
	for (const FTdPath& Path : Map.Paths)
	{
		for (const FTdGridCoord& C : Path.Cells)
		{
			TdGrid::Neighbours4(C, Resolution, Neighbours);
			for (const FTdGridCoord& N : Neighbours)
			{
				if (GetTag(N) == ETdCellTag::Path)
				{
					continue;
				}
				const int32 I = TdGrid::Index(N, Resolution);
				if (Map.Height.IsValidIndex(I))
				{
					Map.Height[I] = FMath::Lerp(Map.Height[I], GetHeight(C), 0.35f);
				}
			}
		}
	}

	float PadSum = 0.f;
	int32 PadCount = 0;
	for (int32 DY = -1; DY <= 1; ++DY)
	{
		for (int32 DX = -1; DX <= 1; ++DX)
		{
			const FTdGridCoord C(Map.TowerCell.X + DX, Map.TowerCell.Y + DY);
			if (!TdGrid::IsInside(C, Resolution))
			{
				continue;
			}
			PadSum += GetHeight(C);
			++PadCount;
		}
	}
	const float PadH = (PadCount > 0) ? (PadSum / PadCount) : 0.f;
	for (int32 DY = -1; DY <= 1; ++DY)
	{
		for (int32 DX = -1; DX <= 1; ++DX)
		{
			const FTdGridCoord C(Map.TowerCell.X + DX, Map.TowerCell.Y + DY);
			if (!TdGrid::IsInside(C, Resolution))
			{
				continue;
			}
			Map.Height[TdGrid::Index(C, Resolution)] = PadH;
			SetTag(C, ETdCellTag::TowerPad);
		}
	}

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const FTdGridCoord C(X, Y);
			const ETdCellTag Existing = GetTag(C);
			if (Existing == ETdCellTag::Path || Existing == ETdCellTag::TowerPad)
			{
				continue;
			}

			float MaxDelta = 0.f;
			TdGrid::Neighbours4(C, Resolution, Neighbours);
			for (const FTdGridCoord& N : Neighbours)
			{
				MaxDelta = FMath::Max(MaxDelta, FMath::Abs(GetHeight(C) - GetHeight(N)));
			}
			SetTag(C, MaxDelta <= SlopeBuildableMax ? ETdCellTag::Buildable : ETdCellTag::Blocked);
		}
	}
}

void AMapGenerator::ExtractSlots()
{
	Map.Slots.Reset();

	TArray<FTdGridCoord> Candidates;
	TArray<FTdGridCoord> Neighbours;

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const FTdGridCoord C(X, Y);
			if (GetTag(C) != ETdCellTag::Buildable)
			{
				continue;
			}

			bool bTouchesPath = false;
			TdGrid::Neighbours4(C, Resolution, Neighbours);
			for (const FTdGridCoord& N : Neighbours)
			{
				if (GetTag(N) == ETdCellTag::Path)
				{
					bTouchesPath = true;
					break;
				}
			}
			if (bTouchesPath)
			{
				Candidates.Add(C);
			}
		}
	}

	FRandomStream SlotRng(Map.Seed ^ 0x51C07);
	for (int32 i = Candidates.Num() - 1; i > 0; --i)
	{
		Candidates.Swap(i, SlotRng.RandRange(0, i));
	}

	const int32 Spacing = FMath::Max(1, SlotSpacing);
	for (const FTdGridCoord& C : Candidates)
	{
		if (Map.Slots.Num() >= MaxSlots)
		{
			break;
		}

		bool bTooClose = false;
		for (const FTdSlotData& Existing : Map.Slots)
		{
			const int32 Dx = FMath::Abs(Existing.Cell.X - C.X);
			const int32 Dy = FMath::Abs(Existing.Cell.Y - C.Y);
			if (FMath::Max(Dx, Dy) < Spacing)
			{
				bTooClose = true;
				break;
			}
		}
		if (bTooClose)
		{
			continue;
		}

		FTdSlotData Slot;
		Slot.Cell = C;
		Slot.WorldTransform = FTransform(GetCellWorld(C));
		Slot.HeightAdvantage = GetHeight(C) - GetHeight(Map.TowerCell);
		Map.Slots.Add(Slot);
	}
}

void AMapGenerator::BuildMesh()
{
	if (!TerrainMesh)
	{
		return;
	}

	const int32 VRes = Resolution + 1;
	const FVector Origin = GetActorLocation();

	TArray<FVector> Verts;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> Colors;
	TArray<int32> Tris;
	TArray<FProcMeshTangent> Tangents;

	Verts.SetNum(VRes * VRes);
	Normals.SetNumZeroed(VRes * VRes);
	UVs.SetNum(VRes * VRes);
	Colors.SetNum(VRes * VRes);
	Tangents.SetNum(VRes * VRes);

	auto CornerIndex = [VRes](int32 Col, int32 Row) { return Row * VRes + Col; };

	auto CornerHeight = [this](int32 Col, int32 Row) -> float
	{
		float Sum = 0.f;
		int32 Count = 0;
		for (int32 DY = -1; DY <= 0; ++DY)
		{
			for (int32 DX = -1; DX <= 0; ++DX)
			{
				const int32 CX = Col + DX;
				const int32 CY = Row + DY;
				if (TdGrid::IsInside(CX, CY, Resolution))
				{
					Sum += Map.Height[TdGrid::Index(CX, CY, Resolution)];
					++Count;
				}
			}
		}
		return (Count > 0) ? (Sum / Count) : 0.f;
	};

	auto CornerColor = [this](int32 Col, int32 Row) -> FColor
	{
		int32 PathN = 0, BuildN = 0, BlockN = 0, PadN = 0;
		for (int32 DY = -1; DY <= 0; ++DY)
		{
			for (int32 DX = -1; DX <= 0; ++DX)
			{
				const int32 CX = Col + DX;
				const int32 CY = Row + DY;
				if (!TdGrid::IsInside(CX, CY, Resolution))
				{
					continue;
				}
				switch (Map.Tags[TdGrid::Index(CX, CY, Resolution)])
				{
				case ETdCellTag::Path: ++PathN; break;
				case ETdCellTag::Buildable: ++BuildN; break;
				case ETdCellTag::TowerPad: ++PadN; break;
				default: ++BlockN; break;
				}
			}
		}
		if (PathN > 0) { return FColor(120, 90, 50); }
		if (PadN > 0) { return FColor(220, 220, 210); }
		if (BuildN >= BlockN) { return FColor(70, 140, 55); }
		return FColor(90, 90, 95);
	};

	for (int32 Row = 0; Row < VRes; ++Row)
	{
		for (int32 Col = 0; Col < VRes; ++Col)
		{
			const int32 I = CornerIndex(Col, Row);
			Verts[I] = Origin + FVector(Col * CellSize, Row * CellSize, CornerHeight(Col, Row));
			UVs[I] = FVector2D(static_cast<float>(Col) / Resolution, static_cast<float>(Row) / Resolution);
			Colors[I] = CornerColor(Col, Row);
			Tangents[I] = FProcMeshTangent(1.f, 0.f, 0.f);
		}
	}

	Tris.Reserve(Resolution * Resolution * 6);
	for (int32 Row = 0; Row < Resolution; ++Row)
	{
		for (int32 Col = 0; Col < Resolution; ++Col)
		{
			const int32 I00 = CornerIndex(Col, Row);
			const int32 I10 = CornerIndex(Col + 1, Row);
			const int32 I01 = CornerIndex(Col, Row + 1);
			const int32 I11 = CornerIndex(Col + 1, Row + 1);
			
			Tris.Add(I00);
			Tris.Add(I10);
			Tris.Add(I01);
			Tris.Add(I10);
			Tris.Add(I11);
			Tris.Add(I01);
		}
	}

	for (int32 T = 0; T < Tris.Num(); T += 3)
	{
		const FVector& A = Verts[Tris[T]];
		const FVector& B = Verts[Tris[T + 1]];
		const FVector& C = Verts[Tris[T + 2]];
		//trying to get the normal to point downwards, so that the mesh is lit correctly
		//reason being the map is currently upside down, so the normals are also flipped
		const FVector N = FVector::CrossProduct(B - A, C - A).GetSafeNormal() * -1.f;
		Normals[Tris[T]] += N;
		Normals[Tris[T + 1]] += N;
		Normals[Tris[T + 2]] += N;
	}
	for (FVector& N : Normals)
	{
		N = N.GetSafeNormal();
		if (N.IsNearlyZero())
		{
			N = FVector::UpVector;
		}
	}

	TerrainMesh->CreateMeshSection(0, Verts, Tris, Normals, UVs, Colors, Tangents, true);

	UMaterialInterface* Mat = TerrainMaterial
		? TerrainMaterial.Get()
		: UMaterial::GetDefaultMaterial(MD_Surface);
	TerrainMesh->SetMaterial(0, Mat);
}

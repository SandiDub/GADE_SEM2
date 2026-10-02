#include "WaveDirector.h"
#include "Actors/EnemySpawner.h"
#include "Actors/Defender.h"
#include "Actors/DefenderSlot.h"
#include "Actors/CentralTower.h"
#include "Combat/HealthComponent.h"
#include "Data/EnemyData.h"
#include "Data/DefenderData.h"
#include "Game/TdGameState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TdTypes.h"

AWaveDirector::AWaveDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWaveDirector::StartMatch(const TArray<AEnemySpawner*>& InSpawners)
{
	Spawners.Reset();
	for (AEnemySpawner* S : InSpawners)
	{
		if (S) { Spawners.Add(S); }
	}

	WaveIndex = 0;
	LastBudget = BaseBudget;
	LastStyle = ETdPlayStyle::Unknown;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaveTimer);
		World->GetTimerManager().SetTimer(WaveTimer, this, &AWaveDirector::OnWaveTimer, WaveInterval, true);
		OnWaveTimer();
	}
}

void AWaveDirector::StopMatch()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaveTimer);
	}
	for (AEnemySpawner* S : Spawners)
	{
		if (S) { S->StopSpawning(); }
	}
}

void AWaveDirector::OnWaveTimer()
{
	FTdPlayerTelemetry T;
	CollectTelemetry(T);
	LastStyle = ClassifyStyle(T);
	T.Style = LastStyle;
	LastBudget = ComputeBudget(T);

	TArray<FTdWaveSpawn> Wave;
	ComposeWave(LastBudget, T, Wave);
	Dispatch(Wave);
	++WaveIndex;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Cyan,
			FString::Printf(TEXT("Wave %d  budget %.0f  style %d  spawns %d"),
				WaveIndex, LastBudget, static_cast<int32>(LastStyle), Wave.Num()));
	}
}

void AWaveDirector::CollectTelemetry(FTdPlayerTelemetry& Out) const
{
	Out = FTdPlayerTelemetry();
	UWorld* World = GetWorld();
	if (!World) { return; }

	if (ATdGameState* GS = World->GetGameState<ATdGameState>())
	{
		Out.Gold = GS->Gold;
		Out.Kills = GS->Kills;
		Out.Leaks = GS->Leaks;
		Out.GoldSpent = GS->GoldSpent;
		Out.Pressure = GS->GetPressure(PressureWindow);
	}

	if (AActor* Tower = UGameplayStatics::GetActorOfClass(World, ACentralTower::StaticClass()))
	{
		if (UHealthComponent* H = Tower->FindComponentByClass<UHealthComponent>())
		{
			Out.TowerHealthNorm = H->GetHealthNormalized();
		}
	}

	TArray<AActor*> Defenders;
	UGameplayStatics::GetAllActorsOfClass(World, ADefender::StaticClass(), Defenders);
	for (AActor* A : Defenders)
	{
		ADefender* D = Cast<ADefender>(A);
		if (!D) { continue; }
		switch (D->GetBehaviour())
		{
		case ETdDefenderBehaviour::Frost:  ++Out.Frosts; break;
		case ETdDefenderBehaviour::Mortar: ++Out.Mortars; break;
		default: ++Out.Gunners; break;
		}
	}

	// Lane coverage comes from slot ownership, which the generator already
	// assigned per path. No distance guessing, so the signal is exact.
	Out.Lanes.SetNum(Spawners.Num());
	for (int32 i = 0; i < Spawners.Num(); ++i)
	{
		Out.Lanes[i].PathIndex = i;
		Out.Lanes[i].DefenderCount = 0;
	}

	TArray<AActor*> Slots;
	UGameplayStatics::GetAllActorsOfClass(World, ADefenderSlot::StaticClass(), Slots);
	for (AActor* A : Slots)
	{
		const ADefenderSlot* Slot = Cast<ADefenderSlot>(A);
		if (!Slot || !Slot->bOccupied) { continue; }

		const int32 Lane = Slot->SlotData.PathIndex;
		if (Out.Lanes.IsValidIndex(Lane))
		{
			++Out.Lanes[Lane].DefenderCount;
		}
	}
}

ETdPlayStyle AWaveDirector::ClassifyStyle(const FTdPlayerTelemetry& T) const
{
	const int32 Total = T.Gunners + T.Frosts + T.Mortars;

	int32 MaxLane = 0;
	int32 SecondLane = 0;
	int32 LanesWithOne = 0;
	int32 LanesWithThreePlus = 0;
	int32 EmptyLanes = 0;

	for (const FTdLaneCoverage& Lane : T.Lanes)
	{
		const int32 C = Lane.DefenderCount;
		if (C > MaxLane)
		{
			SecondLane = MaxLane;
			MaxLane = C;
		}
		else if (C > SecondLane)
		{
			SecondLane = C;
		}

		if (C == 0) { ++EmptyLanes; }
		if (C == 1) { ++LanesWithOne; }
		if (C >= 3) { ++LanesWithThreePlus; }
	}

	(void)EmptyLanes;

	// Spatial turtle: one generated path holds at least twice the next lane.
	if (MaxLane >= 2 && MaxLane >= 2 * SecondLane)
	{
		return ETdPlayStyle::OneLane;
	}

	// Mix: mortars dominate (slow splash, weak vs runners).
	if (T.Mortars >= 1 && T.Mortars >= T.Gunners)
	{
		return ETdPlayStyle::MortarNest;
	}

	// Cheap gunner clumps without splash coverage.
	if (Total >= 4 && T.Mortars == 0 && T.Gunners >= FMath::Max(3, T.Frosts + 1))
	{
		return ETdPlayStyle::CheapSpam;
	}

	if (LanesWithOne >= 2 && LanesWithThreePlus == 0)
	{
		return ETdPlayStyle::SpreadThin;
	}

	return ETdPlayStyle::Unknown;
}

float AWaveDirector::ComputeBudget(const FTdPlayerTelemetry& T) const
{
	// ThreatBudget = Base + (TimeGrowth * MatchTime) + SkillModifier
	const float MatchTime = (GetWorld() && GetWorld()->GetGameState<ATdGameState>())
		? GetWorld()->GetGameState<ATdGameState>()->MatchTime
		: 0.f;

	const float Baseline = BaseBudget + (BudgetPerSecond * MatchTime);

	const bool bLeaking = T.Leaks > 0;
	const bool bHighPressure = T.Pressure >= 0.2f;
	float SkillModifier = 0.f;

	if (bLeaking || bHighPressure)
	{
		// Ease off 20–35% so a struggling player is not steamrolled.
		const float LeakStress = FMath::Clamp(static_cast<float>(T.Leaks) / 4.f, 0.f, 1.f);
		const float Stress = FMath::Clamp(FMath::Max(T.Pressure, LeakStress), 0.f, 1.f);
		const float Slash = FMath::Lerp(0.20f, 0.35f, Stress);
		SkillModifier = -Baseline * Slash;
	}
	else if (T.TowerHealthNorm >= 0.7f && T.Pressure < 0.1f)
	{
		// Stomping: raise budget with spend and kills so challenge stays consistent.
		const float Stomp = FMath::Clamp(
			(static_cast<float>(T.GoldSpent) / 250.f) + (static_cast<float>(T.Kills) / 30.f),
			0.15f, 0.45f);
		SkillModifier = Baseline * Stomp;
	}

	return FMath::Max(8.f, Baseline + SkillModifier);
}

void AWaveDirector::ComposeWave(float Budget, const FTdPlayerTelemetry& T, TArray<FTdWaveSpawn>& Out) const
{
	Out.Reset();

	UEnemyData* Grunt = nullptr;
	UEnemyData* Runner = nullptr;
	UEnemyData* Bruiser = nullptr;
	UEnemyData* Fallback = nullptr;

	for (UEnemyData* E : EnemyRoster)
	{
		if (!E) { continue; }
		if (!Fallback) { Fallback = E; }
		switch (E->Behaviour)
		{
		case ETdEnemyBehaviour::Runner:  if (!Runner) { Runner = E; } break;
		case ETdEnemyBehaviour::Bruiser: if (!Bruiser) { Bruiser = E; } break;
		default: if (!Grunt) { Grunt = E; } break;
		}
	}

	if (!Grunt) { Grunt = Fallback; }
	if (!Grunt) { return; }

	bool bHasEmptyLane = false;
	for (const FTdLaneCoverage& Lane : T.Lanes)
	{
		if (Lane.DefenderCount <= 0)
		{
			bHasEmptyLane = true;
			break;
		}
	}

	// Default mix: Grunts carry the wave. Style and empty lanes shift elites.
	float WGrunt = 1.0f;
	float WRunner = 0.15f;
	float WBruiser = 0.15f;

	if (T.Style == ETdPlayStyle::OneLane || T.Style == ETdPlayStyle::MortarNest || bHasEmptyLane)
	{
		WRunner += 0.85f;
	}
	if (T.Style == ETdPlayStyle::CheapSpam)
	{
		WBruiser += 0.9f;
	}
	if (T.Pressure >= 0.25f || T.Leaks > 0)
	{
		WRunner *= 0.45f;
		WBruiser *= 0.3f;
	}

	if (!Runner) { WRunner = 0.f; }
	if (!Bruiser) { WBruiser = 0.f; }

	auto CostOf = [](UEnemyData* E) -> float
	{
		return (E && E->ThreatCost > 0) ? static_cast<float>(E->ThreatCost) : 10.f;
	};

	const float Cheapest = FMath::Min3(CostOf(Grunt),
		Runner ? CostOf(Runner) : TNumericLimits<float>::Max(),
		Bruiser ? CostOf(Bruiser) : TNumericLimits<float>::Max());

	float Remaining = FMath::Max(Cheapest, Budget);
	int32 Guard = 64;

	while (Remaining + 0.01f >= Cheapest && Guard-- > 0)
	{
		const float Sum = WGrunt + WRunner + WBruiser;
		float Roll = FMath::FRandRange(0.f, FMath::Max(Sum, 0.001f));

		UEnemyData* Pick = Grunt;
		ETdEnemyBehaviour Type = ETdEnemyBehaviour::Grunt;

		if ((Roll -= WRunner) <= 0.f && Runner)
		{
			Pick = Runner;
			Type = ETdEnemyBehaviour::Runner;
		}
		else if ((Roll -= WBruiser) <= 0.f && Bruiser)
		{
			Pick = Bruiser;
			Type = ETdEnemyBehaviour::Bruiser;
		}

		if (CostOf(Pick) > Remaining + 0.01f)
		{
			if (CostOf(Grunt) <= Remaining + 0.01f)
			{
				Pick = Grunt;
				Type = ETdEnemyBehaviour::Grunt;
			}
			else
			{
				break;
			}
		}

		FTdWaveSpawn Spawn;
		Spawn.Enemy = Pick;
		Spawn.PathIndex = PickPath(T, Type);
		Out.Add(Spawn);
		Remaining -= CostOf(Pick);
	}
}

int32 AWaveDirector::PickPath(const FTdPlayerTelemetry& T, ETdEnemyBehaviour ForType) const
{
	const int32 PathCount = Spawners.Num();
	if (PathCount <= 0)
	{
		return 0;
	}

	auto Coverage = [&T, PathCount](int32 Index) -> int32
	{
		if (T.Lanes.IsValidIndex(Index))
		{
			return T.Lanes[Index].DefenderCount;
		}
		return 0;
	};

	if (ForType == ETdEnemyBehaviour::Runner)
	{
		int32 Best = 0;
		int32 BestCount = Coverage(0);
		for (int32 i = 1; i < PathCount; ++i)
		{
			const int32 C = Coverage(i);
			if (C < BestCount || (C == BestCount && (i % FMath::Max(1, PathCount)) == (WaveIndex % PathCount)))
			{
				BestCount = C;
				Best = i;
			}
		}
		return Best;
	}

	if (ForType == ETdEnemyBehaviour::Bruiser)
	{
		int32 Best = 0;
		int32 BestCount = Coverage(0);
		for (int32 i = 1; i < PathCount; ++i)
		{
			const int32 C = Coverage(i);
			if (C > BestCount)
			{
				BestCount = C;
				Best = i;
			}
		}
		return Best;
	}

	return WaveIndex % PathCount;
}

void AWaveDirector::Dispatch(const TArray<FTdWaveSpawn>& Wave)
{
	for (const FTdWaveSpawn& S : Wave)
	{
		if (!S.Enemy || !Spawners.IsValidIndex(S.PathIndex) || !Spawners[S.PathIndex])
		{
			continue;
		}
		// Requires EnemySpawner::SpawnNow(UEnemyData*) — see MERGE notes.
		Spawners[S.PathIndex]->SpawnNow(S.Enemy);
	}
}

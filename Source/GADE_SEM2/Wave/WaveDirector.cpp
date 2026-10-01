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
	// TODO: match your document.
	// OneLane: max(Lane.DefenderCount) >= 2 * second-max, and max >= 2
	// CheapSpam: Gunners high, Mortars == 0, total defenders >= 4
	// MortarNest: Mortars >= Gunners && Mortars >= 1
	// SpreadThin: at least 2 lanes with 1 defender and none with 3+
	(void)T;
	return ETdPlayStyle::Unknown;
}

float AWaveDirector::ComputeBudget(const FTdPlayerTelemetry& T) const
{
	// TODO (yours): finish the rubber-band described in your planning document.
	// Baseline growth is wired so pressure already climbs wave over wave.
	// Still to add: ease off when T.Pressure is high, push when the player stomps.
	float Budget = BaseBudget;

	if (const ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr)
	{
		Budget += BudgetPerSecond * GS->MatchTime;
		Budget *= FMath::Max(1.f, GS->EnemyHealthScale * 0.5f + 0.5f);
	}

	(void)T;
	return FMath::Max(BaseBudget, Budget);
}

void AWaveDirector::ComposeWave(float Budget, const FTdPlayerTelemetry& T, TArray<FTdWaveSpawn>& Out) const
{
	Out.Reset();

	// TODO: spend Budget using EnemyData->ThreatCost.
	// Default fill: Grunts.
	// If style is OneLane or MortarNest: raise Runner weight, PickPath(Runner)
	//   should return the emptiest lane.
	// If style is CheapSpam: raise Bruiser weight.
	// If Pressure high: fewer elites (your rubber-band paragraph).
	//
	// Fallback so the game still plays while you write the real mixer:
	UEnemyData* Fallback = nullptr;
	for (UEnemyData* E : EnemyRoster)
	{
		if (E) { Fallback = E; break; }
	}
	if (!Fallback) { return; }

	int32 Remaining = FMath::Max(1, FMath::RoundToInt(Budget / 10.f));
	for (int32 i = 0; i < Remaining; ++i)
	{
		FTdWaveSpawn S;
		S.Enemy = Fallback;
		S.PathIndex = PickPath(T, ETdEnemyBehaviour::Grunt);
		Out.Add(S);
	}
}

int32 AWaveDirector::PickPath(const FTdPlayerTelemetry& T, ETdEnemyBehaviour ForType) const
{
	if (Spawners.Num() == 0) { return 0; }

	// TODO: Runners -> argmin LaneCoverage
	// Bruisers -> argmax LaneCoverage (hit the stack)
	// Grunts -> round-robin or least-recently-used path
	(void)T;
	(void)ForType;
	return WaveIndex % Spawners.Num();
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

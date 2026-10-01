#include "TdGameState.h"
#include "TdGameMode.h"
#include "Actors/CentralTower.h"
#include "Actors/Defender.h"
#include "Actors/DefenderSlot.h"
#include "Actors/Enemy.h"
#include "Combat/HealthComponent.h"
#include "Data/DefenderData.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	/** The HUD wants "can I afford the next one", which only the GameMode knows. */
	int32 NextCostFor(const UWorld* World)
	{
		const ATdGameMode* GM = World ? World->GetAuthGameMode<ATdGameMode>() : nullptr;
		return GM ? GM->GetNextDefenderCost() : 0;
	}
}

ATdGameState::ATdGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATdGameState::AddGold(int32 Amount)
{
	if (Amount > 0)
	{
		Gold += Amount;
		OnGoldChanged.Broadcast(Gold, NextCostFor(GetWorld()));
	}
}

void ATdGameState::AddPressure(float Amount)
{
	if (Amount > 0.f)
	{
		Pressure = FMath::Clamp(Pressure + Amount, 0.f, 1.f);
	}
}

bool ATdGameState::TrySpendGold(int32 Amount)
{
	if (Amount < 0 || Gold < Amount)
	{
		return false;
	}
	Gold -= Amount;
	GoldSpent += Amount;
	OnGoldChanged.Broadcast(Gold, NextCostFor(GetWorld()));
	return true;
}

void ATdGameState::ResetForNewMatch(int32 InSeed, int32 StartingGold, float InWaveLength)
{
	Seed = InSeed;
	Gold = StartingGold;
	Kills = 0;
	Leaks = 0;
	GoldSpent = 0;
	Pressure = 0.f;
	SelectedDefenderIndex = 0;
	MatchTime = 0.f;

	WaveNumber = 1;
	WaveLength = FMath::Max(5.f, InWaveLength);
	WaveCountdown = WaveLength;
	EnemyHealthScale = 1.f;
	EnemyDamageScale = 1.f;
	EnemySpeedScale = 1.f;
	DefendersPurchased = 0;

	MatchState = ETdMatchState::Playing;

	OnWaveChanged.Broadcast(WaveNumber);
	OnGoldChanged.Broadcast(Gold, NextCostFor(GetWorld()));
}

void ATdGameState::AdvanceWave(float HealthPerWave, float DamagePerWave, float SpeedPerWave, float MaxSpeedScale)
{
	++WaveNumber;

	// Compounding growth. This is what stops defenders out-scaling the enemies.
	EnemyHealthScale *= (1.f + HealthPerWave);
	EnemyDamageScale *= (1.f + DamagePerWave);
	EnemySpeedScale = FMath::Min(MaxSpeedScale, EnemySpeedScale * (1.f + SpeedPerWave));

	OnWaveChanged.Broadcast(WaveNumber);
}

void ATdGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState != ETdMatchState::Playing)
	{
		return;
	}

	MatchTime += DeltaSeconds;
	Pressure = FMath::Max(0.f, Pressure - DeltaSeconds * 0.05f);

	WaveCountdown -= DeltaSeconds;
	if (WaveCountdown <= 0.f)
	{
		WaveCountdown += WaveLength;
		if (ATdGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATdGameMode>() : nullptr)
		{
			GM->NotifyWaveElapsed();
		}
	}
}

FTdHudSnapshot ATdGameState::GetHudSnapshot() const
{
	FTdHudSnapshot Snap;

	Snap.Gold = Gold;
	Snap.Kills = Kills;
	Snap.Leaks = Leaks;
	Snap.MatchTime = MatchTime;
	Snap.Seed = Seed;
	Snap.MatchState = MatchState;
	Snap.WaveNumber = WaveNumber;
	Snap.WaveProgress = (WaveLength > 0.f)
		? FMath::Clamp(1.f - (WaveCountdown / WaveLength), 0.f, 1.f)
		: 0.f;

	UWorld* World = GetWorld();
	if (!World)
	{
		return Snap;
	}

	if (const ATdGameMode* GM = World->GetAuthGameMode<ATdGameMode>())
	{
		Snap.NextDefenderCost = GM->GetNextDefenderCost();
		if (const UDefenderData* Data = GM->GetSelectedDefenderData())
		{
			Snap.SelectedDefenderName = Data->DisplayName;
		}
	}
	Snap.bCanAffordNext = Gold >= Snap.NextDefenderCost;

	if (AActor* Tower = UGameplayStatics::GetActorOfClass(World, ACentralTower::StaticClass()))
	{
		if (const UHealthComponent* H = Tower->FindComponentByClass<UHealthComponent>())
		{
			Snap.TowerHealthPercent = H->GetHealthNormalized();
		}
	}

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemy::StaticClass(), Enemies);
	Snap.EnemiesAlive = Enemies.Num();

	TArray<AActor*> Defenders;
	UGameplayStatics::GetAllActorsOfClass(World, ADefender::StaticClass(), Defenders);
	Snap.DefendersAlive = Defenders.Num();

	// Lane readouts come straight from slot ownership, so the HUD can show
	// "lane 3 is undefended" without any distance guesswork.
	TArray<AActor*> Slots;
	UGameplayStatics::GetAllActorsOfClass(World, ADefenderSlot::StaticClass(), Slots);
	Snap.SlotsTotal = Slots.Num();

	for (AActor* A : Slots)
	{
		const ADefenderSlot* Slot = Cast<ADefenderSlot>(A);
		if (!Slot)
		{
			continue;
		}

		const int32 Lane = Slot->SlotData.PathIndex;
		if (Lane < 0)
		{
			continue;
		}

		if (Snap.LaneSlots.Num() <= Lane)
		{
			Snap.LaneSlots.SetNumZeroed(Lane + 1);
		}
		if (Snap.LaneDefenders.Num() <= Lane)
		{
			Snap.LaneDefenders.SetNumZeroed(Lane + 1);
		}

		++Snap.LaneSlots[Lane];
		if (Slot->bOccupied)
		{
			++Snap.LaneDefenders[Lane];
		}
	}

	return Snap;
}

FText ATdGameState::GetMatchTimeText() const
{
	const int32 Total = FMath::FloorToInt(MatchTime);
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Total / 60, Total % 60));
}

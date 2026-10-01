#include "TdGameMode.h"
#include "TdGameState.h"
#include "TdPawn.h"
#include "TdPlayerController.h"
#include "Terrain/MapGenerator.h"
#include "Actors/EnemySpawner.h"
#include "Actors/Enemy.h"
#include "Actors/Defender.h"
#include "Wave/WaveDirector.h"
#include "Data/DefenderData.h"
#include "Data/EnemyData.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ATdGameMode::ATdGameMode()
{
	GameStateClass = ATdGameState::StaticClass();
	DefaultPawnClass = ATdPawn::StaticClass();
	PlayerControllerClass = ATdPlayerController::StaticClass();
}

void ATdGameMode::BeginPlay()
{
	Super::BeginPlay();
	StartNewMatch();
}

void ATdGameMode::StartNewMatch()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ATdGameState* GS = GetGameState<ATdGameState>();
	const int32 Seed = FMath::Rand();
	CurrentSpawnInterval = SpawnInterval;
	if (GS)
	{
		GS->ResetForNewMatch(Seed, StartingGold, WaveLength);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Td: Game State must be TdGameState. Generation will still run."));
	}

	UGameplayStatics::SetGamePaused(this, false);

	if (!GeneratorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Td: Generator Class is None on the GameMode. Assign BP_MapGenerator."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Red,
				TEXT("Td: set Generator Class = BP_MapGenerator on BP_TdGameMode"));
		}
		return;
	}

	if (!Generator)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Generator = World->SpawnActor<AMapGenerator>(GeneratorClass, FTransform::Identity, Params);
		if (!Generator)
		{
			UE_LOG(LogTemp, Error, TEXT("Td: failed to spawn MapGenerator."));
			return;
		}
	}

	Generator->GenerateWorld(GS ? GS->Seed : Seed);

	TArray<AActor*> SpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemySpawner::StaticClass(), SpawnerActors);
	TArray<AEnemySpawner*> Spawners;
	ActiveSpawners.Reset();
	for (AActor* Actor : SpawnerActors)
	{
		if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(Actor))
		{
			Spawner->SetEnemyClass(EnemyClass);
			Spawners.Add(Spawner);
			ActiveSpawners.Add(Spawner);
		}
	}

	if (WaveDirectorClass)
	{
		if (WaveDirector)
		{
			WaveDirector->StopMatch();
			WaveDirector->Destroy();
			WaveDirector = nullptr;
		}

		FActorSpawnParameters Params;
		Params.Owner = this;
		WaveDirector = World->SpawnActor<AWaveDirector>(WaveDirectorClass, FTransform::Identity, Params);
		if (WaveDirector)
		{
			WaveDirector->EnemyRoster = EnemyRoster.Num() > 0 ? EnemyRoster : TArray<TObjectPtr<UEnemyData>>{ StarterEnemy };
			// One wave clock: the GameState counts waves, the director spawns them.
			WaveDirector->WaveInterval = WaveLength;
			WaveDirector->StartMatch(Spawners);
		}
	}
	else
	{
		for (AEnemySpawner* Spawner : Spawners)
		{
			Spawner->StartSpawning(EnemyClass, StarterEnemy, CurrentSpawnInterval);
		}
	}

	FramePlayerOnTower();

	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ATdGameMode::FramePlayerOnTower);
	}
}

void ATdGameMode::FramePlayerOnTower()
{
	if (!Generator)
	{
		return;
	}

	const FVector TowerLoc = Generator->GetMap().TowerTransform.GetLocation();
	if (ATdPawn* Pawn = Cast<ATdPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Pawn->FrameLocation(TowerLoc);
	}
}

void ATdGameMode::RestartMatch()
{
	// New seed, new map. This is the brief's "different every time you start a new game."
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), Enemies);
	for (AActor* E : Enemies)
	{
		E->Destroy();
	}

	TArray<AActor*> Defenders;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADefender::StaticClass(), Defenders);
	for (AActor* D : Defenders)
	{
		D->Destroy();
	}

	StartNewMatch();
}

void ATdGameMode::SetPausedMatch(bool bPause)
{
	if (ATdGameState* GS = GetGameState<ATdGameState>())
	{
		if (GS->MatchState == ETdMatchState::GameOver)
		{
			return;
		}
		GS->MatchState = bPause ? ETdMatchState::Paused : ETdMatchState::Playing;
	}
	UGameplayStatics::SetGamePaused(this, bPause);
}

void ATdGameMode::NotifyTowerDestroyed()
{
	if (ATdGameState* GS = GetGameState<ATdGameState>())
	{
		GS->MatchState = ETdMatchState::GameOver;
	}

	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawner::StaticClass(), Spawners);
	for (AActor* Actor : Spawners)
	{
		if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(Actor))
		{
			Spawner->StopSpawning();
		}
	}

	if (WaveDirector)
	{
		WaveDirector->StopMatch();
	}

	// TODO (Day 7): show a game-over UMG widget (survived time, kills, Restart button).
}

int32 ATdGameMode::GetNextDefenderCost() const
{
	const UDefenderData* Data = GetSelectedDefenderData();
	const int32 Base = Data ? Data->Cost : 50;

	int32 Purchased = 0;
	if (const ATdGameState* GS = GetGameState<ATdGameState>())
	{
		Purchased = GS->DefendersPurchased;
	}

	return Base + Purchased * DefenderCostStep;
}

void ATdGameMode::NotifyDefenderPurchased()
{
	if (ATdGameState* GS = GetGameState<ATdGameState>())
	{
		++GS->DefendersPurchased;
	}
}

void ATdGameMode::NotifyWaveElapsed()
{
	ATdGameState* GS = GetGameState<ATdGameState>();
	if (!GS || GS->MatchState != ETdMatchState::Playing)
	{
		return;
	}

	GS->AdvanceWave(EnemyHealthPerWave, EnemyDamagePerWave, EnemySpeedPerWave, MaxEnemySpeedScale);
	GS->AddGold(GoldPerWave);

	// The director owns cadence once it exists; otherwise tighten the Part 1 spawners.
	if (!WaveDirector)
	{
		CurrentSpawnInterval = FMath::Max(MinSpawnInterval, CurrentSpawnInterval * SpawnIntervalPerWave);
		for (AEnemySpawner* Spawner : ActiveSpawners)
		{
			if (Spawner)
			{
				Spawner->StartSpawning(EnemyClass, StarterEnemy, CurrentSpawnInterval);
			}
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("Wave %d  HP x%.2f  DMG x%.2f  interval %.2fs"),
				GS->WaveNumber, GS->EnemyHealthScale, GS->EnemyDamageScale, CurrentSpawnInterval));
	}
}

void ATdGameMode::SetSelectedDefenderIndex(int32 Index)
{
	if (ATdGameState* GS = GetGameState<ATdGameState>())
	{
		const int32 MaxIndex = FMath::Max(0, DefenderRoster.Num() - 1);
		GS->SelectedDefenderIndex = FMath::Clamp(Index, 0, MaxIndex);
	}
}

UDefenderData* ATdGameMode::GetSelectedDefenderData() const
{
	if (ATdGameState* GS = GetGameState<ATdGameState>())
	{
		if (DefenderRoster.IsValidIndex(GS->SelectedDefenderIndex))
		{
			return DefenderRoster[GS->SelectedDefenderIndex];
		}
	}
	return StarterDefender;
}

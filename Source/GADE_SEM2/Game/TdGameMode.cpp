#include "TdGameMode.h"
#include "TdGameState.h"
#include "TdPawn.h"
#include "TdPlayerController.h"
#include "Terrain/MapGenerator.h"
#include "Actors/EnemySpawner.h"
#include "Actors/Enemy.h"
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
	if (GS)
	{
		GS->Gold = StartingGold;
		GS->Kills = 0;
		GS->MatchTime = 0.f;
		GS->Seed = Seed;
		GS->MatchState = ETdMatchState::Playing;
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

	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemySpawner::StaticClass(), Spawners);
	for (AActor* Actor : Spawners)
	{
		if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(Actor))
		{
			Spawner->StartSpawning(EnemyClass, StarterEnemy, SpawnInterval);
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

	// TODO (Day 7): show a game-over UMG widget (survived time, kills, Restart button).
}

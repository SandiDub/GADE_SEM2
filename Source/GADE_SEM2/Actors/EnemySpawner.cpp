#include "EnemySpawner.h"
#include "Enemy.h"
#include "Data/EnemyData.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::Configure(const FTdPath& InPath)
{
	Path = InPath;
	if (Path.Waypoints.Num() > 0)
	{
		SetActorLocation(Path.Waypoints[0]);
	}
}

void AEnemySpawner::StartSpawning(TSubclassOf<AEnemy> EnemyClass, UEnemyData* Data, float InitialInterval)
{
	CachedEnemyClass = EnemyClass;
	CachedData = Data;

	// Set our starting speed
	CurrentSpawnRate = InitialInterval;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimer);

		// Set looping to FALSE so we can manually re-trigger it at a faster rate
		World->GetTimerManager().SetTimer(SpawnTimer, this, &AEnemySpawner::SpawnOne, CurrentSpawnRate, false);
	}
}

void AEnemySpawner::StopSpawning()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimer);
	}
}

void AEnemySpawner::SpawnOne()
{
	if (!CachedEnemyClass || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector Loc = Path.Waypoints.Num() > 0 ? Path.Waypoints[0] : GetActorLocation();
	AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(CachedEnemyClass, Loc, FRotator::ZeroRotator, Params);
	if (Enemy)
	{
		Enemy->ApplyData(CachedData);
		Enemy->SetPath(Path);
	}

	// Increment our tracker
	EnemiesSpawned++;

	// Check if we hit a multiple of 10!
	if (EnemiesSpawned % WaveThreshold == 0)
	{
		// Drop the spawn rate by 0.75s, but don't go below the 1.5s minimum
		CurrentSpawnRate = FMath::Max(MinimumSpawnRate, CurrentSpawnRate - SpawnRateDecrease);

		
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("Wave Escaping! New Spawn Rate: %f"), CurrentSpawnRate));
	}

	// Set the NEW timer 
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AEnemySpawner::SpawnOne, CurrentSpawnRate, false);
}
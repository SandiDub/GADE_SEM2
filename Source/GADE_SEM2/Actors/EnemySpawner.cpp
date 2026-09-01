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

void AEnemySpawner::StartSpawning(TSubclassOf<AEnemy> EnemyClass, UEnemyData* Data, float Interval)
{
	CachedEnemyClass = EnemyClass;
	CachedData = Data;
	SpawnInterval = Interval;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimer);
		World->GetTimerManager().SetTimer(SpawnTimer, this, &AEnemySpawner::SpawnOne, SpawnInterval, true);
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
}

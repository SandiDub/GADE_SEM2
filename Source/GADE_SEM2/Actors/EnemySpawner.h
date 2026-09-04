#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "EnemySpawner.generated.h"

class AEnemy;
class UEnemyData;

UCLASS()
class GADE_SEM2_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	void Configure(const FTdPath& InPath);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void StartSpawning(TSubclassOf<AEnemy> EnemyClass, UEnemyData* Data, float InitialInterval);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void StopSpawning();

protected:
	void SpawnOne();

	FTdPath Path;

	TSubclassOf<AEnemy> CachedEnemyClass;

	UPROPERTY()
	TObjectPtr<UEnemyData> CachedData;

	FTimerHandle SpawnTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float CurrentSpawnRate = 18.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float MinimumSpawnRate = 6.0f; // The absolute fastest they can spawn

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnRateDecrease = 0.75f; // Shaves 0.75s off the timer per threshold

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 WaveThreshold = 8; // Number of enemies before the spawn speeds up

	int32 EnemiesSpawned = 0; // Silently tracks total spawned in the background
};
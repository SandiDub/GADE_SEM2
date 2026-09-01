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
	void StartSpawning(TSubclassOf<AEnemy> EnemyClass, UEnemyData* Data, float Interval);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void StopSpawning();

protected:
	void SpawnOne();

	FTdPath Path;

	UPROPERTY(EditAnywhere, Category = "Td")
	float SpawnInterval = 2.5f;

	TSubclassOf<AEnemy> CachedEnemyClass;

	UPROPERTY()
	TObjectPtr<UEnemyData> CachedData;

	FTimerHandle SpawnTimer;
};

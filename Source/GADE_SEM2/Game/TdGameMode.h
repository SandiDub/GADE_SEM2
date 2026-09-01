#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TdGameMode.generated.h"

class AMapGenerator;
class AEnemySpawner;
class AEnemy;
class UDefenderData;
class UEnemyData;

UCLASS()
class GADE_SEM2_API ATdGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATdGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<AMapGenerator> GeneratorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Classes")
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Data")
	TObjectPtr<UDefenderData> StarterDefender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Data")
	TObjectPtr<UEnemyData> StarterEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance")
	int32 StartingGold = 80;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Balance", meta = (ClampMin = "0.5"))
	float SpawnInterval = 2.5f;

	UFUNCTION(BlueprintCallable, Category = "Td")
	void StartNewMatch();

	UFUNCTION(BlueprintCallable, Category = "Td")
	void RestartMatch();

	UFUNCTION(BlueprintCallable, Category = "Td")
	void SetPausedMatch(bool bPause);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void NotifyTowerDestroyed();

protected:
	virtual void BeginPlay() override;

	void FramePlayerOnTower();

	UPROPERTY()
	TObjectPtr<AMapGenerator> Generator;
};

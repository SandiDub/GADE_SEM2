#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyData.generated.h"

class UStaticMesh;

/** One row of enemy stats. Part 2 = extra assets, not extra C++ classes. */
UCLASS(BlueprintType)
class GADE_SEM2_API UEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy", meta = (ClampMin = "1.0"))
	float MaxHealth = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy", meta = (ClampMin = "0.0"))
	float Damage = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy", meta = (ClampMin = "0.05"))
	float AttackInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy", meta = (ClampMin = "1.0"))
	float MoveSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy", meta = (ClampMin = "0.0"))
	float AggroRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy", meta = (ClampMin = "0"))
	int32 GoldReward = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy")
	TObjectPtr<UStaticMesh> Mesh;
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Wave/WaveTypes.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy")
	ETdEnemyBehaviour Behaviour = ETdEnemyBehaviour::Grunt;

	/** Spent from the director ThreatBudget when this unit is queued. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy")
	int32 ThreatCost = 10;

	/**
	 * Pushed into the mesh material as a vector parameter at spawn. Keep enemies
	 * in the red/orange half of the wheel so threat reads instantly.
	 * Values above 1 bloom, which is what makes them glow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy|Visuals")
	FLinearColor AccentColor = FLinearColor(3.f, 0.15f, 0.1f);

	/** Uniform scale applied at spawn. Cheap way to give each type a silhouette. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Enemy|Visuals", meta = (ClampMin = "0.1"))
	FVector MeshScale = FVector(1.f, 1.f, 1.f);
};

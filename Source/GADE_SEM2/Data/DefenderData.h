#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DefenderData.generated.h"

class UStaticMesh;
class USkeletalMesh;

/** One row of defender stats. Part 2 = extra assets, not extra C++ classes. */
UCLASS(BlueprintType)
class GADE_SEM2_API UDefenderData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender", meta = (ClampMin = "0"))
	int32 Cost = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender", meta = (ClampMin = "1.0"))
	float MaxHealth = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender", meta = (ClampMin = "0.0"))
	float Damage = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender", meta = (ClampMin = "0.0"))
	float Range = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender", meta = (ClampMin = "0.05"))
	float FireInterval = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender")
	TObjectPtr<UStaticMesh> Mesh;
};

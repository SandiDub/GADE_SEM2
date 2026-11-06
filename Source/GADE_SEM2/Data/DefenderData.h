#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Wave/WaveTypes.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender")
	ETdDefenderBehaviour Behaviour = ETdDefenderBehaviour::Gunner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender")
	float SplashRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender")
	float SlowMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender")
	float SlowDuration = 0.f;

	/**
	 * Pushed into the mesh material as a vector parameter at spawn. Keep
	 * defenders in the blue/cyan half so friend and foe never read alike.
	 * Values above 1 bloom, which is what makes them glow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender|Visuals")
	FLinearColor AccentColor = FLinearColor(0.1f, 1.2f, 3.f);

	/** Uniform scale applied at spawn. Cheap way to give each type a silhouette. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Defender|Visuals", meta = (ClampMin = "0.1"))
	FVector MeshScale = FVector(1.f, 1.f, 1.f);
};

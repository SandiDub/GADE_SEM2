#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "Wave/WaveTypes.h"
#include "Defender.generated.h"

class UHealthComponent;
class UAutoAttackComponent;
class UStaticMeshComponent;
class UDefenderData;

UCLASS()
class GADE_SEM2_API ADefender : public AActor
{
	GENERATED_BODY()

public:
	ADefender();

	UPROPERTY(VisibleAnywhere, Category = "Td")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
	TObjectPtr<UAutoAttackComponent> AutoAttack;

	UFUNCTION(BlueprintPure, Category = "Td")
	ETdTeam GetTeam() const { return ETdTeam::Player; }

	void ApplyData(UDefenderData* Data);
	void ApplyHeightAdvantage(float HeightAdvantage);

	UFUNCTION(BlueprintPure, Category = "Td")
	ETdDefenderBehaviour GetBehaviour() const { return CachedBehaviour; }

	UFUNCTION(BlueprintPure, Category = "Td")
	UDefenderData* GetData() const { return CachedData; }

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Td")
	float HeightToRangeScale = 0.35f;

	/** Hard cap on the high-ground bonus so one ridge slot cannot cover all lanes. */
	UPROPERTY(EditAnywhere, Category = "Td")
	float MaxRangeBonus = 150.f;

	UFUNCTION()
	void HandleDeath();

	UPROPERTY()
	TObjectPtr<UDefenderData> CachedData;

	ETdDefenderBehaviour CachedBehaviour = ETdDefenderBehaviour::Gunner;
};

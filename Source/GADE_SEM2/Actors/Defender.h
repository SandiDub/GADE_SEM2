#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
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

protected:
	UPROPERTY(EditAnywhere, Category = "Td")
	float HeightToRangeScale = 0.5f;
};

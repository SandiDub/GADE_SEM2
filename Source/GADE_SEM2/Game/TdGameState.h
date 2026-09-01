#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TdTypes.h"
#include "TdGameState.generated.h"

UCLASS()
class GADE_SEM2_API ATdGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ATdGameState();
	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Seed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Gold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	ETdMatchState MatchState = ETdMatchState::Waiting;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	float MatchTime = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Td")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Td")
	bool TrySpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Td")
	void AddKill() { ++Kills; }

	UFUNCTION(BlueprintPure, Category = "Td")
	bool CanAfford(int32 Amount) const { return Gold >= Amount; }

	virtual void Tick(float DeltaSeconds) override;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TdTypes.h"
#include "AutoAttackComponent.generated.h"

class AActor;

/**
 * Shared auto-fire for tower and defenders (and optionally enemies).
 * Fill in AcquireTarget and Fire — those are the marks.
 */
UCLASS(ClassGroup = (TdGame), meta = (BlueprintSpawnableComponent))
class GADE_SEM2_API UAutoAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoAttackComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Combat")
	ETdTeam Team = ETdTeam::Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Combat", meta = (ClampMin = "0.0"))
	float Range = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Combat", meta = (ClampMin = "0.05"))
	float FireInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Combat", meta = (ClampMin = "0.0"))
	float Damage = 8.f;

	/** Extra range from slot height. Set by ADefender after placement. */
	UPROPERTY(BlueprintReadWrite, Category = "Td|Combat")
	float RangeBonus = 0.f;

	UFUNCTION(BlueprintPure, Category = "Td|Combat")
	float GetEffectiveRange() const { return Range + RangeBonus; }

	UFUNCTION(BlueprintCallable, Category = "Td|Combat")
	void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** TODO: sphere overlap, filter by opposing team + UHealthComponent, pick closest (or first in range). */
	AActor* AcquireTarget() const;

	/** TODO: apply Damage to the target's UHealthComponent. Optional: spawn a projectile later. */
	void FireAt(AActor* Target);

	UPROPERTY(VisibleAnywhere, Category = "Td|Combat")
	TWeakObjectPtr<AActor> CurrentTarget;

	float CooldownRemaining = 0.f;
	bool bEnabled = true;
};

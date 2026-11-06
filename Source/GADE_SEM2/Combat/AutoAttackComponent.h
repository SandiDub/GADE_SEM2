#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TdTypes.h"
#include "AutoAttackComponent.generated.h"

class AActor;

/**
 * Fired once per volley, before damage. Bind in BP_Defender / BP_CentralTower
 * and spawn a Niagara beam between MuzzleLocation and HitLocation.
 * One short-lived burst per shot is far cheaper than real projectile actors.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTdFired, AActor*, Target, FVector, MuzzleLocation, FVector, HitLocation);

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

	/** Hook your muzzle flash / tracer VFX here. */
	UPROPERTY(BlueprintAssignable, Category = "Td|Combat")
	FOnTdFired OnFired;

	/** Red editor tracer. Turn this off once your Niagara beam is bound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Combat")
	bool bDrawDebugTracer = false;

	/** Local offset the beam starts from, so shots leave the barrel not the floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Combat")
	FVector MuzzleOffset = FVector(0.f, 0.f, 90.f);

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

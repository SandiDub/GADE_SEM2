#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTdHealthChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTdDeath);

/**
 * Shared HP for tower, defenders, and enemies.
 * Bind OnHealthChanged to a Widget Component bar.
 * Bind OnDeath to GameMode (tower) or gold reward (enemy) or slot-free (defender).
 */
UCLASS(ClassGroup = (TdGame), meta = (BlueprintSpawnableComponent))
class GADE_SEM2_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintAssignable, Category = "Td|Health")
	FOnTdHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Td|Health")
	FOnTdDeath OnDeath;

	UFUNCTION(BlueprintPure, Category = "Td|Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Td|Health")
	float GetHealthNormalized() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Td|Health")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintCallable, Category = "Td|Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "Td|Health")
	void TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Td|Health")
	void Heal(float Amount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Td|Health")
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Td|Health")
	bool bDead = false;
};

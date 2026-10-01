#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "TimerManager.h"
#include "Enemy.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
class UEnemyData;
class AActor;

UCLASS()
class GADE_SEM2_API AEnemy : public AActor
{
	GENERATED_BODY()

public:
	AEnemy();

	UPROPERTY(VisibleAnywhere, Category = "Td")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
	TObjectPtr<UHealthComponent> Health;

	UFUNCTION(BlueprintPure, Category = "Td")
	ETdTeam GetTeam() const { return ETdTeam::Enemy; }

	void ApplyData(UEnemyData* Data);
	void SetPath(const FTdPath& InPath);

	void ApplySlow(float Multiplier, float Duration);

	UFUNCTION()
	void ClearSlow();

	/** Base stat x wave scale x slow. Use these, never Data->MoveSpeed directly. */
	UFUNCTION(BlueprintPure, Category = "Td")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Td")
	float GetAttackDamage() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleDeath();

	/** TODO: walk waypoints. If a defender is in AggroRange, attack it, then resume. At the end, attack the tower. */
	void AdvanceAlongPath(float DeltaTime);
	void TryAttack(float DeltaTime);

	UPROPERTY()
	TObjectPtr<UEnemyData> Data;

	FTdPath Path;
	int32 WaypointIndex = 0;

	UPROPERTY()
	TWeakObjectPtr<AActor> AttackTarget;

	float AttackCooldown = 0.f;

	float MoveSpeedMultiplier = 1.f;
	FTimerHandle SlowTimer;
	bool bHasLeaked = false;

	/** Captured from GameState at spawn so a unit keeps the wave it arrived in. */
	float HealthScale = 1.f;
	float DamageScale = 1.f;
	float SpeedScale = 1.f;
};

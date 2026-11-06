#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "Wave/WaveTypes.h"
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

	/** Vector parameter on the mesh material that receives EnemyData::AccentColor. */
	UPROPERTY(EditAnywhere, Category = "Td|Visuals")
	FName TintParamName = TEXT("BaseTint");

	// ---- procedural mutation ----
	// Custom procedural generation feature. Every enemy rolls its own body
	// plan and stat spread at spawn from a seeded stream, so no two units are
	// identical and the roster keeps producing new silhouettes all match.
	// Each mutation trades a stat away for the one it gains, so the roll
	// changes how a unit must be answered without changing its threat value.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Mutation")
	bool bEnableMutation = true;

	/** Resolved at spawn. Shown on the health bar so the player can read the roll. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|Mutation")
	FText MutationName;

	UPROPERTY(BlueprintReadOnly, Category = "Td|Mutation")
	ETdEnemyMutation Mutation = ETdEnemyMutation::None;

	/** Fires after the roll so Blueprints can add a matching aura or decal. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Td|Mutation")
	void OnMutationApplied(ETdEnemyMutation InMutation);

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

	/**
	 * Rolls this unit's appearance and stat spread. Called from ApplyData,
	 * after the base stats land and before the material is built.
	 */
	void ApplyProceduralMutation(FLinearColor& InOutTint, FVector& InOutScale);

	/** Per-instance multipliers written by the mutation roll. */
	float MutationDamageScale = 1.f;
	float MutationSpeedScale = 1.f;

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

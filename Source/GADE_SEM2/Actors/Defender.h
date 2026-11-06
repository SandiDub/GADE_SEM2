#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "Wave/WaveTypes.h"
#include "Actors/TdClickable.h"
#include "Defender.generated.h"

class UHealthComponent;
class UAutoAttackComponent;
class UStaticMeshComponent;
class UDefenderData;
class UMaterialInstanceDynamic;
class USoundBase;

UCLASS()
class GADE_SEM2_API ADefender : public AActor, public ITdClickable
{
	GENERATED_BODY()

public:
	ADefender();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
    int32 CurrentLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrades")
    int32 MaxLevel = 2;

    /** Lets the UI grey out the button and the controller avoid charging for a no-op. */
    UFUNCTION(BlueprintPure, Category = "Upgrades")
    bool CanUpgrade() const { return CurrentLevel < MaxLevel; }

    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    void Upgrade();

    UFUNCTION(BlueprintImplementableEvent, Category = "Upgrades")
    void OnUpgraded();

	// ---- per-instance upgrade modifiers ----
	// The data asset is shared by every defender of a type, so upgrades layer
	// on top of it here rather than mutating the asset.

	/** Added to the data asset's SplashRadius. */
	UPROPERTY(BlueprintReadOnly, Category = "Upgrades")
	float SplashRadiusBonus = 0.f;

	/** Added to the data asset's SlowDuration. Gives slow to types that had none. */
	UPROPERTY(BlueprintReadOnly, Category = "Upgrades")
	float BonusSlowDuration = 0.f;

	/** Multiplies the data asset's SlowMultiplier. Below 1 means a harsher slow. */
	UPROPERTY(BlueprintReadOnly, Category = "Upgrades")
	float SlowStrengthScale = 1.f;

	/** Extra enemies struck per volley beyond the primary target. */
	UPROPERTY(BlueprintReadOnly, Category = "Upgrades")
	int32 ExtraTargets = 0;

	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetEffectiveSplashRadius() const;

	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetEffectiveSlowDuration() const;

	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetEffectiveSlowMultiplier() const;

	// ---- visuals ----
	// Every defender gets its own Dynamic Material Instance, so recolouring one
	// never touches the others and upgrade glow is per-unit.

	/** Vector parameter on the mesh material that receives DefenderData::AccentColor. */
	UPROPERTY(EditAnywhere, Category = "Td|Visuals")
	FName TintParamName = TEXT("BaseTint");

	/** Scalar parameter that receives CurrentLevel so the material can brighten. */
	UPROPERTY(EditAnywhere, Category = "Td|Visuals")
	FName LevelParamName = TEXT("UpgradeLevel");

	UPROPERTY(BlueprintReadOnly, Category = "Td|Visuals")
	TObjectPtr<UMaterialInstanceDynamic> BodyMID;

	/** Played at the death location. Set this in BP_Defender. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Audio")
	TObjectPtr<USoundBase> DeathSound;

	/** Clicking a built defender selects it as the upgrade target. */
	virtual bool OnClickedByPlayer(ATdPlayerController* Player) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
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

	/** Behaviour-specific half of an upgrade. Runs after the shared stat scaling. */
	void ApplyUpgradeEffects();

	/** Builds BodyMID and pushes the accent colour and current level into it. */
	void RefreshMaterial();

	UFUNCTION()
	void HandleDeath();

	UPROPERTY()
	TObjectPtr<UDefenderData> CachedData;

	ETdDefenderBehaviour CachedBehaviour = ETdDefenderBehaviour::Gunner;	
};

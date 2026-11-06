#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "Actors/TdClickable.h"
#include "CentralTower.generated.h"

class UHealthComponent;
class UAutoAttackComponent;
class UStaticMeshComponent;

UCLASS()
class GADE_SEM2_API ACentralTower : public AActor, public ITdClickable
{
	GENERATED_BODY()

public:
	ACentralTower();
	
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

	/** Clicking the hub selects it as the upgrade target. */
	virtual bool OnClickedByPlayer(ATdPlayerController* Player) override;

	// ---- visuals ----
	// The hub has no data asset, so its colour lives here. Keep it the deepest
	// blue on the map: it should read as the thing you are protecting.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Visuals")
	FLinearColor AccentColor = FLinearColor(0.2f, 0.6f, 4.f);

	UPROPERTY(EditAnywhere, Category = "Td|Visuals")
	FName TintParamName = TEXT("BaseTint");

	UPROPERTY(EditAnywhere, Category = "Td|Visuals")
	FName LevelParamName = TEXT("UpgradeLevel");

	UPROPERTY(BlueprintReadOnly, Category = "Td|Visuals")
	TObjectPtr<class UMaterialInstanceDynamic> BodyMID;

	/** The hub's own destruction sound. Set this in BP_CentralTower. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Td|Audio")
	TObjectPtr<class USoundBase> DeathSound;

	void RefreshMaterial();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Td")
	TObjectPtr<UAutoAttackComponent> AutoAttack;

	UFUNCTION(BlueprintPure, Category = "Td")
	ETdTeam GetTeam() const { return ETdTeam::Player; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	float LastHealth = 0.f;
};

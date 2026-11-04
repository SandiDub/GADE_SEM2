#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "CentralTower.generated.h"

class UHealthComponent;
class UAutoAttackComponent;
class UStaticMeshComponent;

UCLASS()
class GADE_SEM2_API ACentralTower : public AActor
{
	GENERATED_BODY()

public:
	ACentralTower();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
    int32 CurrentLevel = 0;

    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    void Upgrade();

    UFUNCTION(BlueprintImplementableEvent, Category = "Upgrades")
    void OnUpgraded();

	UPROPERTY(VisibleAnywhere, Category = "Td")
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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "TdPlayerController.generated.h"

/** Fires when the player clicks a different defender or the tower. Drives the upgrade panel. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTdUpgradeTargetChanged, AActor*, Target);

UCLASS()
class GADE_SEM2_API ATdPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATdPlayerController();

	/** Defender or tower the player last clicked. Null when nothing is selected. */
	UPROPERTY(BlueprintReadOnly, Category = "Td|Upgrades")
	TObjectPtr<AActor> SelectedUpgradeTarget;

	UPROPERTY(BlueprintAssignable, Category = "Td|Upgrades")
	FOnTdUpgradeTargetChanged OnUpgradeTargetChanged;

	UFUNCTION(BlueprintCallable, Category = "Td|Upgrades")
	void SetUpgradeTarget(AActor* Target);

	/**
	 * Spend gold and upgrade the selected actor. Returns false without charging
	 * when the target is already at max level or the player cannot afford it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Td|Upgrades")
	bool TryUpgradeActor(AActor* SelectedActor, int32 UpgradeCost);

	/** Level readout for the upgrade panel. Returns false if the actor is not upgradeable. */
	UFUNCTION(BlueprintPure, Category = "Td|Upgrades")
	bool GetUpgradeInfo(AActor* Target, int32& OutLevel, int32& OutMaxLevel, bool& bOutCanUpgrade) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Input")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Input")
	TObjectPtr<UInputAction> PanAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Td|Input")
	TObjectPtr<UInputAction> ZoomAction;

	void HandleZoom(const FInputActionValue& Value);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void HandleClick(const FInputActionValue& Value);
	void HandlePause(const FInputActionValue& Value);
	void HandlePan(const FInputActionValue& Value);

	void SelectDefenderSlot(int32 Index);
	void SelectDefender0() { SelectDefenderSlot(0); }
	void SelectDefender1() { SelectDefenderSlot(1); }
	void SelectDefender2() { SelectDefenderSlot(2); }
};

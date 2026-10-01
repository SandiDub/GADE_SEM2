#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "TdPlayerController.generated.h"

UCLASS()
class GADE_SEM2_API ATdPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATdPlayerController();

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

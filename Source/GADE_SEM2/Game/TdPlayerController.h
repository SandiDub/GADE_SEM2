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

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void HandleClick(const FInputActionValue& Value);
	void HandlePause(const FInputActionValue& Value);
	void HandlePan(const FInputActionValue& Value);
};

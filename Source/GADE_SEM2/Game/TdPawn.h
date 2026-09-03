#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TdPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;

/** Top-down observer. The player does not walk; they click slots. */
UCLASS()
class GADE_SEM2_API ATdPawn : public APawn
{
	GENERATED_BODY()

public:
	ATdPawn();

	UPROPERTY(VisibleAnywhere, Category = "Td")
	TObjectPtr<USceneComponent> Pivot;

	UPROPERTY(VisibleAnywhere, Category = "Td")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Td")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Td|Camera")
	float PanSpeed = 1200.f;

	void Pan(FVector2D Axis);

	UPROPERTY(EditAnywhere, Category = "Td|Camera")
	float ZoomSpeed = 200.f;

	void Zoom(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Td|Camera")
	void FrameLocation(FVector WorldLocation);

protected:
	virtual void BeginPlay() override;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdTypes.h"
#include "TdClickable.h"
#include "DefenderSlot.generated.h"

class ADefender;
class UDefenderData;
class UStaticMeshComponent;

UCLASS()
class GADE_SEM2_API ADefenderSlot : public AActor, public ITdClickable
{
	GENERATED_BODY()

public:
	ADefenderSlot();

	UPROPERTY(VisibleAnywhere, Category = "Td")
	TObjectPtr<UStaticMeshComponent> MarkerMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	FTdSlotData SlotData;

	UPROPERTY(BlueprintReadOnly, Category = "Td")
	bool bOccupied = false;

	void Configure(const FTdSlotData& InData);

	virtual bool OnClickedByPlayer(class ATdPlayerController* Player) override;

protected:
	UPROPERTY()
	TObjectPtr<ADefender> OccupyingDefender;

	UFUNCTION()
	void HandleDefenderDeath();
};

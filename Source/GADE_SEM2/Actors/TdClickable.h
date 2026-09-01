#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TdClickable.generated.h"

class ATdPlayerController;

UINTERFACE(BlueprintType)
class GADE_SEM2_API UTdClickable : public UInterface
{
	GENERATED_BODY()
};

class GADE_SEM2_API ITdClickable
{
	GENERATED_BODY()

public:
	virtual bool OnClickedByPlayer(ATdPlayerController* Player) = 0;
};

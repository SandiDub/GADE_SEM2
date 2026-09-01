#include "TdGameState.h"

ATdGameState::ATdGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATdGameState::AddGold(int32 Amount)
{
	if (Amount > 0)
	{
		Gold += Amount;
	}
}

bool ATdGameState::TrySpendGold(int32 Amount)
{
	if (Amount < 0 || Gold < Amount)
	{
		return false;
	}
	Gold -= Amount;
	return true;
}

void ATdGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MatchState == ETdMatchState::Playing)
	{
		MatchTime += DeltaSeconds;
	}
}

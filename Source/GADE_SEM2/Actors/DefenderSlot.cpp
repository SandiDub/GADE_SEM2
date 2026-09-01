#include "DefenderSlot.h"
#include "Defender.h"
#include "Data/DefenderData.h"
#include "Game/TdPlayerController.h"
#include "Game/TdGameState.h"
#include "Game/TdGameMode.h"
#include "Combat/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ADefenderSlot::ADefenderSlot()
{
	PrimaryActorTick.bCanEverTick = false;

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	SetRootComponent(MarkerMesh);
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MarkerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	MarkerMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ADefenderSlot::Configure(const FTdSlotData& InData)
{
	SlotData = InData;
	SetActorTransform(InData.WorldTransform);
}

bool ADefenderSlot::OnClickedByPlayer(ATdPlayerController* Player)
{
	(void)Player;

	if (bOccupied)
	{
		return false;
	}

	ATdGameMode* GM = Cast<ATdGameMode>(UGameplayStatics::GetGameMode(this));
	ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr;
	if (!GM || !GS)
	{
		return false;
	}

	UDefenderData* Data = GM->StarterDefender;
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("Td: assign StarterDefender on the GameMode."));
		return false;
	}

	// TODO: You write the buy rules.
	// 1. If GS cannot afford Data->Cost, return false (play a deny sound later).
	// 2. Spend gold.
	// 3. Spawn ADefender at this transform, ApplyData(Data), ApplyHeightAdvantage(SlotData.HeightAdvantage).
	// 4. Bind the defender's OnDeath to HandleDefenderDeath so the slot frees.
	// 5. Hide or dim MarkerMesh. Set bOccupied = true.
	//
	// Paths are never slots — the generator already excluded them. Do not add a
	// second "can I place on grass" raycast here.

	return false;
}

void ADefenderSlot::HandleDefenderDeath()
{
	bOccupied = false;
	OccupyingDefender = nullptr;
	if (MarkerMesh)
	{
		MarkerMesh->SetVisibility(true);
	}
}

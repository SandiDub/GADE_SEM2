#include "DefenderSlot.h"
#include "Defender.h"
#include "Data/DefenderData.h"
#include "Game/TdPlayerController.h"
#include "Game/TdGameState.h"
#include "Game/TdGameMode.h"
#include "Combat/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

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
    if (bOccupied) return false;

    ATdGameMode* GM = Cast<ATdGameMode>(UGameplayStatics::GetGameMode(this));
    ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr;

    if (!GM || !GS) return false;

    UDefenderData* Data = GM->GetSelectedDefenderData();
    if (!Data)
    {
        Data = GM->StarterDefender;
    }
    if (!Data)
    {
        UE_LOG(LogTemp, Warning, TEXT("Td: Assign StarterDefender on BP_TdGameMode!"));
        return false;
    }

    if (!GM->DefenderClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Td: Assign DefenderClass on BP_TdGameMode!"));
        return false;
    }

    // Cost escalates with every defender bought, so spamming turrets is a real decision.
    const int32 Price = GM->GetNextDefenderCost();
    if (!GS->CanAfford(Price))
    {
        UE_LOG(LogTemp, Warning, TEXT("Td: Not enough gold (need %d, have %d)."), Price, GS->Gold);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
                FString::Printf(TEXT("Need %d gold"), Price));
        }
        return false;
    }

    GS->TrySpendGold(Price);

    //Spawn the Blueprint version of the Defender
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    OccupyingDefender = GetWorld()->SpawnActor<ADefender>(GM->DefenderClass, SlotData.WorldTransform, SpawnParams);

    //Apply data and bind delegates
    if (OccupyingDefender)
    {
        OccupyingDefender->ApplyData(Data);
        OccupyingDefender->ApplyHeightAdvantage(SlotData.HeightAdvantage);

        // Bind the defender's OnDeath to HandleDefenderDeath so the slot frees
        OccupyingDefender->Health->OnDeath.AddDynamic(this, &ADefenderSlot::HandleDefenderDeath);

        // Hide the MarkerMesh and set bOccupied = true
        if (MarkerMesh)
        {
            MarkerMesh->SetVisibility(false,true);
        }
        bOccupied = true;
        GM->NotifyDefenderPurchased();

        return true;
    }

    return false;
}

void ADefenderSlot::HandleDefenderDeath()
{
    bOccupied = false;
    OccupyingDefender = nullptr;
    if (MarkerMesh)
    {
        MarkerMesh->SetVisibility(true, true);
    }

}

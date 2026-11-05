#include "CentralTower.h"
#include "Combat/HealthComponent.h"
#include "Combat/AutoAttackComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/TdGameMode.h"
#include "Game/TdGameState.h"
#include "Game/TdPlayerController.h"

ACentralTower::ACentralTower()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->MaxHealth = 100.f;

	AutoAttack = CreateDefaultSubobject<UAutoAttackComponent>(TEXT("AutoAttack"));
	AutoAttack->Team = ETdTeam::Player;
	AutoAttack->Range = 700.f;
	AutoAttack->FireInterval = 1.f;
	AutoAttack->Damage = 8.f;
}

void ACentralTower::BeginPlay()
{
	Super::BeginPlay();
	Health->OnDeath.AddDynamic(this, &ACentralTower::HandleDeath);
	Health->OnHealthChanged.AddDynamic(this, &ACentralTower::HandleHealthChanged);
	LastHealth = Health->GetHealth();
}

void ACentralTower::HandleHealthChanged(float Current, float Max)
{
	if (Current < LastHealth && Max > 0.f)
	{
		if (ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr)
		{
			GS->AddPressure((LastHealth - Current) / Max);
		}
	}
	LastHealth = Current;
}

void ACentralTower::HandleDeath()
{
	if (ATdGameMode* GM = Cast<ATdGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->NotifyTowerDestroyed();
	}
}

bool ACentralTower::OnClickedByPlayer(ATdPlayerController* Player)
{
    if (!Player)
    {
        return false;
    }

    Player->SetUpgradeTarget(this);
    return true;
}

void ACentralTower::Upgrade()
{
    // Three visual states total: Base, Upgrade 1, Upgrade 2.
    if (CanUpgrade())
    {
        CurrentLevel++;

        // 1. Scale Health
        if (Health)
        {
            Health->MaxHealth *= 1.5f;
            Health->ResetHealth(); 
        }

        // 2. Scale Attack Speed
        if (AutoAttack)
        {
            // Lower FireInterval means faster shooting
            AutoAttack->FireInterval *= 0.8f; 
        }

        // 3. The hub has no behaviour type, so it upgrades into raw reach and
        //    punch. This is what makes a late tower upgrade worth the gold.
        if (AutoAttack)
        {
            AutoAttack->Range += 120.f;
            AutoAttack->Damage *= 1.25f;
        }

        // 4. Trigger the visual Shader hook for Blueprints
        OnUpgraded();
    }
}
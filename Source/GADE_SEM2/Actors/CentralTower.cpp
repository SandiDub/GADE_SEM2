#include "CentralTower.h"
#include "Combat/HealthComponent.h"
#include "Combat/AutoAttackComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/TdGameMode.h"
#include "Game/TdGameState.h"

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

void ACentralTower::Upgrade()
{
    // Max level is 2 (giving you 3 total visual states: Base, Upgrade 1, Upgrade 2)
    if (CurrentLevel < 2)
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

        // 3. Trigger the visual Shader hook for Blueprints
        OnUpgraded();
    }
}
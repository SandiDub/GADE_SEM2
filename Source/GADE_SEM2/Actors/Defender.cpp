#include "Defender.h"
#include "Combat/HealthComponent.h"
#include "Combat/AutoAttackComponent.h"
#include "Data/DefenderData.h"
#include "Components/StaticMeshComponent.h"

void ADefender::BeginPlay()
{
	Super::BeginPlay();

	// Bind the death event 
	if (Health)
	{
		Health->OnDeath.AddDynamic(this, &ADefender::HandleDeath);
	}
}

ADefender::ADefender()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	AutoAttack = CreateDefaultSubobject<UAutoAttackComponent>(TEXT("AutoAttack"));
	AutoAttack->Team = ETdTeam::Player;
}

void ADefender::ApplyData(UDefenderData* Data)
{
	if (!Data)
	{
		return;
	}

	CachedData = Data;
	CachedBehaviour = Data->Behaviour;

	Health->MaxHealth = Data->MaxHealth;
	Health->ResetHealth();
	AutoAttack->Damage = Data->Damage;
	AutoAttack->Range = Data->Range;
	AutoAttack->FireInterval = Data->FireInterval;

	if (Data->Mesh)
	{
		Mesh->SetStaticMesh(Data->Mesh);
	}
}

void ADefender::ApplyHeightAdvantage(float HeightAdvantage)
{
	// High ground = extra range. Clamped so a ridge slot cannot cover every lane at once,
	// which is what let a single well-placed defender trivialise the map.
	const float Bonus = FMath::Max(0.f, HeightAdvantage) * HeightToRangeScale;
	AutoAttack->RangeBonus = FMath::Clamp(Bonus, 0.f, MaxRangeBonus);
}
void ADefender::HandleDeath()
{
	// You can spawn a particle effect or play a sound here later!
	Destroy(); // Removes the defender from the map
}

void ADefender::Upgrade()
{
    // Max level is 2 (giving you 3 total visual states: Base, Upgrade 1, Upgrade 2)
    if (CurrentLevel < 2)
    {
        CurrentLevel++;

        // 1. Scale Health
        if (Health)
        {
            Health->MaxHealth *= 1.5f;
            Health->ResetHealth(); // Fills them back up to the new max 
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

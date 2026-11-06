#include "Defender.h"
#include "Combat/HealthComponent.h"
#include "Combat/AutoAttackComponent.h"
#include "Data/DefenderData.h"
#include "Components/StaticMeshComponent.h"
#include "Game/TdPlayerController.h"

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
	Mesh->SetRelativeScale3D(Data->MeshScale);

	// Must run after SetStaticMesh: swapping the mesh resets materials.
	RefreshMaterial();
}

void ADefender::RefreshMaterial()
{
	if (!Mesh || !Mesh->GetStaticMesh())
	{
		return;
	}

	if (!BodyMID)
	{
		BodyMID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (!BodyMID)
	{
		return;
	}

	if (CachedData)
	{
		BodyMID->SetVectorParameterValue(TintParamName, CachedData->AccentColor);
	}
	BodyMID->SetScalarParameterValue(LevelParamName, static_cast<float>(CurrentLevel));
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

void ADefender::ApplyUpgradeEffects()
{
    if (!AutoAttack)
    {
        return;
    }

    switch (CachedBehaviour)
    {
    case ETdDefenderBehaviour::Gunner:
        // Reach first, then a second barrel. Stays single-target damage but
        // starts clearing the trash that used to slip past.
        AutoAttack->Range += 90.f;
        if (CurrentLevel >= 2)
        {
            ExtraTargets = 1;
        }
        break;

    case ETdDefenderBehaviour::Frost:
        // The chill bites harder each level, then spreads to the whole group.
        SlowStrengthScale *= 0.8f;
        BonusSlowDuration += 0.5f;
        if (CurrentLevel >= 2)
        {
            SplashRadiusBonus += 260.f;
        }
        break;

    case ETdDefenderBehaviour::Mortar:
        // Wider blast, then the blast also staggers what it does not kill.
        SplashRadiusBonus += 90.f;
        if (CurrentLevel >= 2)
        {
            BonusSlowDuration += 0.8f;
            SlowStrengthScale = FMath::Min(SlowStrengthScale, 0.7f);
        }
        break;
    }
}

float ADefender::GetEffectiveSplashRadius() const
{
    const float Base = CachedData ? CachedData->SplashRadius : 0.f;
    return Base + SplashRadiusBonus;
}

float ADefender::GetEffectiveSlowDuration() const
{
    const float Base = CachedData ? CachedData->SlowDuration : 0.f;
    return Base + BonusSlowDuration;
}

float ADefender::GetEffectiveSlowMultiplier() const
{
    const float Base = CachedData ? CachedData->SlowMultiplier : 1.f;
    // Floored so a stacked Frost never freezes enemies outright.
    return FMath::Clamp(Base * SlowStrengthScale, 0.25f, 1.f);
}

bool ADefender::OnClickedByPlayer(ATdPlayerController* Player)
{
    if (!Player)
    {
        return false;
    }

    Player->SetUpgradeTarget(this);
    return true;
}

void ADefender::Upgrade()
{
    // Three visual states total: Base, Upgrade 1, Upgrade 2.
    if (CanUpgrade())
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

        // 3. Behaviour-specific effect, so each type upgrades into something
        //    mechanically different rather than just a bigger health bar.
        ApplyUpgradeEffects();

        // 4. Push the new level into this unit's own material instance.
        RefreshMaterial();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
                FString::Printf(TEXT("%s upgraded to L%d"),
                    CachedData ? *CachedData->GetName() : TEXT("Defender"), CurrentLevel));
        }

        // 5. Trigger the visual Shader hook for Blueprints
        OnUpgraded();
    }
}

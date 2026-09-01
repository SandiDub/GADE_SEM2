#include "CentralTower.h"
#include "Combat/HealthComponent.h"
#include "Combat/AutoAttackComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/TdGameMode.h"

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
}

void ACentralTower::HandleDeath()
{
	if (ATdGameMode* GM = Cast<ATdGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->NotifyTowerDestroyed();
	}
}

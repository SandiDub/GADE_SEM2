#include "Enemy.h"
#include "Combat/HealthComponent.h"
#include "Data/EnemyData.h"
#include "Game/TdGameState.h"
#include "Components/StaticMeshComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	Health->OnDeath.AddDynamic(this, &AEnemy::HandleDeath);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AdvanceAlongPath(DeltaTime);
	TryAttack(DeltaTime);
}

void AEnemy::ApplyData(UEnemyData* InData)
{
	Data = InData;
	if (!Data)
	{
		return;
	}

	Health->MaxHealth = Data->MaxHealth;
	Health->ResetHealth();
	if (Data->Mesh)
	{
		Mesh->SetStaticMesh(Data->Mesh);
	}
}

void AEnemy::SetPath(const FTdPath& InPath)
{
	Path = InPath;
	WaypointIndex = 0;
	if (Path.Waypoints.Num() > 0)
	{
		SetActorLocation(Path.Waypoints[0]);
	}
}

void AEnemy::AdvanceAlongPath(float DeltaTime)
{
	// TODO: You write this. Do NOT use Recast.
	//
	// 1. If AttackTarget is valid and still in AggroRange, do not walk — TryAttack handles it.
	// 2. Else if WaypointIndex < Path.Waypoints.Num():
	//      MoveTowards waypoint at Data->MoveSpeed * DeltaTime.
	//      If distance < acceptance (e.g. 20 uu), ++WaypointIndex.
	// 3. Optional: sphere overlap for defenders in Data->AggroRange. If found, AttackTarget = that defender.
	// 4. When waypoints are exhausted, AttackTarget = the central tower (find by class).
	(void)DeltaTime;
}

void AEnemy::TryAttack(float DeltaTime)
{
	AttackCooldown -= DeltaTime;
	if (!AttackTarget.IsValid() || !Data)
	{
		return;
	}

	// TODO: If AttackCooldown <= 0, TakeDamage(Data->Damage) on the target's HealthComponent.
	// Reset AttackCooldown = Data->AttackInterval.
	// If the target dies, clear AttackTarget so walking resumes.
}

void AEnemy::HandleDeath()
{
	if (ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr)
	{
		const int32 Reward = Data ? Data->GoldReward : 0;
		GS->AddGold(Reward);
		GS->AddKill();
	}
	Destroy();
}

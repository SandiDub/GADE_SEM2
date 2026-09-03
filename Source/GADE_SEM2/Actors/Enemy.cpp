#include "Enemy.h"
#include "Combat/HealthComponent.h"
#include "Data/EnemyData.h"
#include "Game/TdGameState.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/CentralTower.h"
#include "Actors/Defender.h"

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
{	//If we already have a target, check if it's still alive and in range
    if (AttackTarget.IsValid() && Data)
    {
        float Dist = FVector::Dist(GetActorLocation(), AttackTarget->GetActorLocation());
        if (Dist <= Data->AggroRange)
        {
            return; // Target is close! Stop walking and let TryAttack() handle it
        }
        else
        {
            AttackTarget.Reset(); // Target is too far, forget it and resume walking
        }
    }

    // Scan for nearby Defenders that are placed
    if (!AttackTarget.IsValid() && Data)
    {
        TArray<AActor*> OverlappedActors;

        // Scan a small sphere around the enemy, ONLY looking for placed Defenders
        UKismetSystemLibrary::SphereOverlapActors(
            GetWorld(),
            GetActorLocation(),
            Data->AggroRange,
            TArray<TEnumAsByte<EObjectTypeQuery>>(),
            ADefender::StaticClass(),
            TArray<AActor*>(),
            OverlappedActors
        );

        // If we bump into a defender, lock onto it and stop walking
        if (OverlappedActors.Num() > 0)
        {
            AttackTarget = OverlappedActors[0];
            return;
        }
    }

    // 3. Move towards the next waypoint
    if (WaypointIndex < Path.Waypoints.Num())
    {
        FVector CurrentLoc = GetActorLocation();
        FVector TargetLoc = Path.Waypoints[WaypointIndex];

        // Remove the TargetLoc.Z flattening line! We want them to move up/down hills in 3D.

        FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal();

        if (Data)
        {
            // CHANGE THIS TO FALSE: Turn off sweeping so they don't snag on terrain slopes!
            AddActorWorldOffset(Direction * Data->MoveSpeed * DeltaTime, false);
        }

        // Keep the generous distance check
        if (FVector::Dist(CurrentLoc, TargetLoc) < 50.f)
        {
            WaypointIndex++;
        }
    }
}

void AEnemy::TryAttack(float DeltaTime)
{
	    //If we have a target, check if it is still in range. If so, stop walking.
        if (AttackTarget.IsValid() && Data)
        {
            float Dist = FVector::Dist(GetActorLocation(), AttackTarget->GetActorLocation());
            if (Dist <= Data->AggroRange)
            {
                return; // TryAttack() will handle the fighting
            }
            else
            {
                AttackTarget.Reset(); // Target ran away or died, resume walking
            }
        }

    //Move towards the next waypoint
    if (WaypointIndex < Path.Waypoints.Num())
    {
        FVector CurrentLoc = GetActorLocation();
        FVector TargetLoc = Path.Waypoints[WaypointIndex];

        // Get the direction to the waypoint
        FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal();

        // Move the enemy
        if (Data)
        {
            AddActorWorldOffset(Direction * Data->MoveSpeed * DeltaTime, true);
        }

        // If we are close enough to the waypoint, target the next one
        if (FVector::Dist(CurrentLoc, TargetLoc) < 20.f)
        {
            WaypointIndex++;
        }
    }
    else
    {
        //We reached the end of the path! Attack the Central Tower.
        if (!AttackTarget.IsValid())
        {
            AActor* Tower = UGameplayStatics::GetActorOfClass(GetWorld(), ACentralTower::StaticClass());
            if (Tower)
            {
                AttackTarget = Tower;
            }
        }
    }
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

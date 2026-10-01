#include "Enemy.h"
#include "Combat/HealthComponent.h"
#include "Data/EnemyData.h"
#include "Game/TdGameState.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/CentralTower.h"
#include "Actors/Defender.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Wave/WaveTypes.h"

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

    // Wave scaling is applied at spawn so later waves genuinely out-grow a static defence.
    HealthScale = 1.f;
    DamageScale = 1.f;
    SpeedScale = 1.f;
    if (const ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr)
    {
        HealthScale = GS->EnemyHealthScale;
        DamageScale = GS->EnemyDamageScale;
        SpeedScale = GS->EnemySpeedScale;
    }

    Health->MaxHealth = Data->MaxHealth * HealthScale;
    Health->ResetHealth();
    if (Data->Mesh)
    {
        Mesh->SetStaticMesh(Data->Mesh);
    }
}

float AEnemy::GetMoveSpeed() const
{
    return Data ? Data->MoveSpeed * SpeedScale * MoveSpeedMultiplier : 0.f;
}

float AEnemy::GetAttackDamage() const
{
    return Data ? Data->Damage * DamageScale : 0.f;
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
    const ETdEnemyBehaviour Behaviour = Data ? Data->Behaviour : ETdEnemyBehaviour::Grunt;

    //If we already have a target, check if it's still alive and in range
    if (AttackTarget.IsValid() && Data)
    {
        float Dist = FVector::Dist(GetActorLocation(), AttackTarget->GetActorLocation());
        float Reach = AttackTarget->IsA(ACentralTower::StaticClass()) ? 400.f : Data->AggroRange;

        if (Dist <= Reach)
        {
            return; // Target is close! Stop walking and let TryAttack() handle it
        }
        else
        {
            AttackTarget.Reset(); // Target is too far, forget it and resume walking
        }
    }

    // Runner: never peel defenders. Bruiser/Grunt: scan for them.
    if (Behaviour != ETdEnemyBehaviour::Runner && !AttackTarget.IsValid() && Data)
    {
        TArray<AActor*> OverlappedActors;

        const float ScanRange = Behaviour == ETdEnemyBehaviour::Bruiser ? Data->AggroRange * 1.6f : Data->AggroRange;

        UKismetSystemLibrary::SphereOverlapActors(
            GetWorld(),
            GetActorLocation(),
            ScanRange,
            TArray<TEnumAsByte<EObjectTypeQuery>>(),
            ADefender::StaticClass(),
            TArray<AActor*>(),
            OverlappedActors
        );

        if (OverlappedActors.Num() > 0)
        {
            AttackTarget = OverlappedActors[0];
            return;
        }
    }

    if (WaypointIndex < Path.Waypoints.Num())
    {
        FVector CurrentLoc = GetActorLocation();
        FVector TargetLoc = Path.Waypoints[WaypointIndex];
        FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal();

        if (Data)
        {
            AddActorWorldOffset(Direction * GetMoveSpeed() * DeltaTime, false);
        }

        if (FVector::Dist(CurrentLoc, TargetLoc) < 50.f)
        {
            WaypointIndex++;
        }
    }
    else
    {
        if (!AttackTarget.IsValid())
        {
            AActor* Tower = UGameplayStatics::GetActorOfClass(GetWorld(), ACentralTower::StaticClass());
            if (Tower)
            {
                AttackTarget = Tower;
                if (!bHasLeaked)
                {
                    bHasLeaked = true;
                    if (ATdGameState* GS = GetWorld()->GetGameState<ATdGameState>())
                    {
                        GS->RegisterLeak();
                    }
                }
            }
        }
    }
}

void AEnemy::ApplySlow(float Multiplier, float Duration)
{
    MoveSpeedMultiplier = FMath::Clamp(Multiplier, 0.1f, 1.f);
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SlowTimer);
        World->GetTimerManager().SetTimer(SlowTimer, this, &AEnemy::ClearSlow, Duration, false);
    }
}

void AEnemy::ClearSlow()
{
    MoveSpeedMultiplier = 1.f;
}

void AEnemy::TryAttack(float DeltaTime)
{
    // Check if we have a valid target
    if (AttackTarget.IsValid())
    {
        // Try to grab the target's Health Component
        if (UHealthComponent* TargetHealth = AttackTarget->FindComponentByClass<UHealthComponent>())
        {
            // Ensure we only deal damage if the tower/defender is still alive!
            if (!TargetHealth->IsDead() && Data)
            {
                DrawDebugLine(GetWorld(), GetActorLocation(), AttackTarget->GetActorLocation(), FColor::Yellow, false, 0.2f, 0, 3.f);

                float Dealt = GetAttackDamage() * DeltaTime;
                if (Data->Behaviour == ETdEnemyBehaviour::Bruiser && AttackTarget->IsA(ADefender::StaticClass()))
                {
                    Dealt *= 1.75f;
                }
                TargetHealth->TakeDamage(Dealt);
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
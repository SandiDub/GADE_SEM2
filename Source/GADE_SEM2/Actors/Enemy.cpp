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

    FLinearColor Tint = Data->AccentColor;
    FVector Scale = Data->MeshScale;

    ApplyProceduralMutation(Tint, Scale);

    Mesh->SetRelativeScale3D(Scale);

    // Own material instance per enemy, so the mutation colour is per-unit and
    // recolouring one never bleeds into the rest of the wave.
    if (Mesh->GetStaticMesh())
    {
        if (UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(0))
        {
            MID->SetVectorParameterValue(TintParamName, Tint);
        }
    }

    OnMutationApplied(Mutation);
}

void AEnemy::ApplyProceduralMutation(FLinearColor& InOutTint, FVector& InOutScale)
{
    if (!bEnableMutation || !Data)
    {
        return;
    }

    // Seeded per unit rather than per wave: the stream is derived from this
    // actor's unique name and the wave it arrived in, so the roll is stable
    // for the lifetime of the unit but never repeats across the match.
    const ATdGameState* GS = GetWorld() ? GetWorld()->GetGameState<ATdGameState>() : nullptr;
    const int32 Wave = GS ? GS->WaveNumber : 1;
    FRandomStream Rng(GetFName().GetNumber() * 2654435761u ^ (Wave * 40503));

    // Continuous roll first. Even an unmutated unit gets its own build, which
    // is what stops a wave reading as a row of identical clones.
    const float SizeRoll = Rng.FRandRange(0.82f, 1.25f);
    InOutScale *= SizeRoll;

    // Independent girth-vs-height jitter on top, so even two units that rolled
    // the same size still read as different builds.
    InOutScale.X *= Rng.FRandRange(0.9f, 1.15f);
    InOutScale.Y = InOutScale.X;
    InOutScale.Z *= Rng.FRandRange(0.85f, 1.2f);

    // Bigger bodies carry more mass, smaller ones move quicker.
    Health->MaxHealth *= FMath::Lerp(0.85f, 1.2f, (SizeRoll - 0.82f) / 0.43f);
    MutationSpeedScale *= FMath::Lerp(1.12f, 0.9f, (SizeRoll - 0.82f) / 0.43f);

    // Hue drift keeps every enemy inside the red danger band but distinct.
    InOutTint.G *= Rng.FRandRange(0.6f, 1.5f);
    InOutTint.R *= Rng.FRandRange(0.85f, 1.3f);

    // Discrete mutation second. Later waves mutate more often, so the roster
    // visibly diversifies as pressure builds.
    const float MutateChance = FMath::Min(0.65f, 0.15f + Wave * 0.05f);
    if (Rng.FRand() > MutateChance)
    {
        MutationName = FText::GetEmpty();
        Mutation = ETdEnemyMutation::None;
    }
    else
    {
        static const ETdEnemyMutation Options[] = {
            ETdEnemyMutation::Armoured, ETdEnemyMutation::Swift,
            ETdEnemyMutation::Volatile, ETdEnemyMutation::Giant,
            ETdEnemyMutation::Runt };

        Mutation = Options[Rng.RandRange(0, UE_ARRAY_COUNT(Options) - 1)];

        switch (Mutation)
        {
        // Each mutation reshapes the body rather than just resizing it, so the
        // roll is readable from the silhouette alone at top-down distance.
        case ETdEnemyMutation::Armoured:
            Health->MaxHealth *= 1.6f;
            MutationSpeedScale *= 0.75f;
            InOutScale *= FVector(1.3f, 1.3f, 0.8f);   // squat and broad
            InOutTint = FLinearColor(1.4f, 0.5f, 0.2f);
            break;

        case ETdEnemyMutation::Swift:
            Health->MaxHealth *= 0.65f;
            MutationSpeedScale *= 1.5f;
            InOutScale *= FVector(0.75f, 0.75f, 1.35f); // narrow and tall
            InOutTint = FLinearColor(3.2f, 0.9f, 0.1f);
            break;

        case ETdEnemyMutation::Volatile:
            Health->MaxHealth *= 0.7f;
            MutationDamageScale *= 1.8f;
            InOutScale *= FVector(1.15f, 1.15f, 1.15f); // swollen
            InOutTint = FLinearColor(4.f, 0.05f, 0.6f);
            break;

        case ETdEnemyMutation::Giant:
            Health->MaxHealth *= 2.2f;
            MutationSpeedScale *= 0.6f;
            InOutScale *= FVector(1.7f, 1.7f, 1.4f);
            InOutTint = FLinearColor(2.2f, 0.1f, 0.05f);
            break;

        case ETdEnemyMutation::Runt:
            Health->MaxHealth *= 0.5f;
            MutationSpeedScale *= 1.35f;
            MutationDamageScale *= 0.7f;
            InOutScale *= FVector(0.6f, 0.6f, 0.8f);
            InOutTint = FLinearColor(2.6f, 1.2f, 0.2f);
            break;

        default:
            break;
        }

        MutationName = StaticEnum<ETdEnemyMutation>()->GetDisplayNameTextByValue(
            static_cast<int64>(Mutation));
    }

    Health->ResetHealth();
}

float AEnemy::GetMoveSpeed() const
{
    return Data ? Data->MoveSpeed * SpeedScale * MutationSpeedScale * MoveSpeedMultiplier : 0.f;
}

float AEnemy::GetAttackDamage() const
{
    return Data ? Data->Damage * DamageScale * MutationDamageScale : 0.f;
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
        GS->AddKillReward(Reward);
        GS->AddKill();
    }
    Destroy();
}
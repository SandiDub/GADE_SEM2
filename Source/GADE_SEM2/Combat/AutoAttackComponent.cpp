#include "AutoAttackComponent.h"
#include "HealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actors/Enemy.h"
#include "Actors/Defender.h"
#include "Data/DefenderData.h"
#include "Kismet/GameplayStatics.h"

UAutoAttackComponent::UAutoAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAutoAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	CooldownRemaining = 0.f;
}

void UAutoAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnabled)
	{
		return;
	}

	CooldownRemaining -= DeltaTime;

	if (!CurrentTarget.IsValid() || FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation()) > GetEffectiveRange())
	{
		CurrentTarget = AcquireTarget();
	}

	if (CurrentTarget.IsValid() && CooldownRemaining <= 0.f)
	{
		FireAt(CurrentTarget.Get());
		CooldownRemaining = FireInterval;
	}
}

AActor* UAutoAttackComponent::AcquireTarget() const
{
    if (Team != ETdTeam::Player)
    {
        return nullptr;
    }

    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(GetOwner());
    TArray<AActor*> OverlappedActors;

    // Filter to enemies in the query itself rather than scanning every actor in range.
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        GetEffectiveRange(),
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AEnemy::StaticClass(),
        IgnoredActors,
        OverlappedActors
    );

    AActor* BestTarget = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    for (AActor* Actor : OverlappedActors)
    {
        const UHealthComponent* TargetHealth = Actor->FindComponentByClass<UHealthComponent>();
        if (!TargetHealth || TargetHealth->IsDead())
        {
            continue;
        }

        const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Dist < BestScore)
        {
            BestScore = Dist;
            BestTarget = Actor;
        }
    }

    return BestTarget;
}

void UAutoAttackComponent::FireAt(AActor* Target)
{
    if (!Target) return;

    TArray<AActor*> Victims;
    Victims.Add(Target);

    // Upgrades live on the instance, not the shared data asset, so read the
    // effective values from the defender rather than from DefenderData.
    ADefender* OwnerDef = Cast<ADefender>(GetOwner());
    float SlowMult = 1.f;
    float SlowDuration = 0.f;

    if (OwnerDef)
    {
        SlowMult = OwnerDef->GetEffectiveSlowMultiplier();
        SlowDuration = OwnerDef->GetEffectiveSlowDuration();

        const float Splash = OwnerDef->GetEffectiveSplashRadius();
        if (Splash > 0.f)
        {
            TArray<AActor*> SplashHits;
            UKismetSystemLibrary::SphereOverlapActors(
                GetWorld(),
                Target->GetActorLocation(),
                Splash,
                TArray<TEnumAsByte<EObjectTypeQuery>>(),
                AEnemy::StaticClass(),
                TArray<AActor*>(),
                SplashHits
            );
            for (AActor* Hit : SplashHits)
            {
                Victims.AddUnique(Hit);
            }
        }

        // Upgraded Gunners engage additional enemies inside their own radius.
        if (OwnerDef->ExtraTargets > 0)
        {
            TArray<AActor*> Nearby;
            UKismetSystemLibrary::SphereOverlapActors(
                GetWorld(),
                GetOwner()->GetActorLocation(),
                GetEffectiveRange(),
                TArray<TEnumAsByte<EObjectTypeQuery>>(),
                AEnemy::StaticClass(),
                TArray<AActor*>{ GetOwner() },
                Nearby
            );

            int32 Added = 0;
            for (AActor* Extra : Nearby)
            {
                if (Added >= OwnerDef->ExtraTargets)
                {
                    break;
                }
                if (Extra != Target && Victims.AddUnique(Extra) != INDEX_NONE)
                {
                    ++Added;
                }
            }
        }
    }

    for (AActor* Victim : Victims)
    {
        if (UHealthComponent* TargetHealth = Victim->FindComponentByClass<UHealthComponent>())
        {
            TargetHealth->TakeDamage(Damage);
        }
        if (SlowDuration > 0.f)
        {
            if (AEnemy* E = Cast<AEnemy>(Victim))
            {
                E->ApplySlow(SlowMult, SlowDuration);
            }
        }
    }

    const FVector Muzzle = GetOwner()->GetActorLocation() + MuzzleOffset;
    const FVector Impact = Target->GetActorLocation();

    OnFired.Broadcast(Target, Muzzle, Impact);

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, Muzzle, FireSoundVolume);
    }

    if (bDrawDebugTracer)
    {
        DrawDebugLine(GetWorld(), Muzzle, Impact, FColor::Red, false, 0.2f, 0, 2.f);
    }
}

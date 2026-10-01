#include "AutoAttackComponent.h"
#include "HealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actors/Enemy.h"
#include "Actors/Defender.h"
#include "Data/DefenderData.h"

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

    UDefenderData* DefData = nullptr;
    if (ADefender* OwnerDef = Cast<ADefender>(GetOwner()))
    {
        DefData = OwnerDef->GetData();
        if (DefData && DefData->SplashRadius > 0.f)
        {
            TArray<AActor*> SplashHits;
            UKismetSystemLibrary::SphereOverlapActors(
                GetWorld(),
                Target->GetActorLocation(),
                DefData->SplashRadius,
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
    }

    for (AActor* Victim : Victims)
    {
        if (UHealthComponent* TargetHealth = Victim->FindComponentByClass<UHealthComponent>())
        {
            TargetHealth->TakeDamage(Damage);
        }
        if (DefData && DefData->SlowDuration > 0.f)
        {
            if (AEnemy* E = Cast<AEnemy>(Victim))
            {
                E->ApplySlow(DefData->SlowMultiplier, DefData->SlowDuration);
            }
        }
    }

    DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Target->GetActorLocation(), FColor::Red, false, 0.2f, 0, 2.f);
}

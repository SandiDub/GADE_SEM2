#include "AutoAttackComponent.h"
#include "HealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actors/Enemy.h"

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
	TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(GetOwner());
    TArray<AActor*> OverlappedActors;

    //Scan a sphere around the tower
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        GetEffectiveRange(),
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AActor::StaticClass(),
        IgnoredActors,
        OverlappedActors
    );

    AActor* ClosestTarget = nullptr;
    float ClosestDist = TNumericLimits<float>::Max();

    //Loop through everything we hit to find the closest enemy
    for (AActor* Actor : OverlappedActors)
    {
        if (UHealthComponent* TargetHealth = Actor->FindComponentByClass<UHealthComponent>())
        {
            if (!TargetHealth->IsDead())
            {
                // Ensure the tower only shoots at enemies, not other defenders
                if (Team == ETdTeam::Player && Actor->IsA(AEnemy::StaticClass()))
                {
                    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                    if (Dist < ClosestDist)
                    {
                        ClosestDist = Dist;
                        ClosestTarget = Actor;
                    }
                }
            }
        }
    }
    return ClosestTarget;
	
}

void UAutoAttackComponent::FireAt(AActor* Target)
{
    if (!Target) return;

    if (UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>())
    {
        TargetHealth->TakeDamage(Damage);

        // Draw a red laser beam for 0.2 seconds so we can visually see the tower shooting!
        DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Target->GetActorLocation(), FColor::Red, false, 0.2f, 0, 2.f);
    }
}

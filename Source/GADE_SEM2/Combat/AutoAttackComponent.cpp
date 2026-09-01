#include "AutoAttackComponent.h"
#include "HealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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
	// TODO: You write this. Suggested steps:
	// 1. Sphere overlap at owner location, radius = GetEffectiveRange().
	//    UKismetSystemLibrary::SphereOverlapActors is the simple version.
	// 2. Skip self and dead actors (UHealthComponent::IsDead).
	// 3. Keep only the opposing team. Add a team getter on tower/defender/enemy,
	//    or a tiny UTeamComponent if you prefer.
	// 4. Return the closest remaining actor, or nullptr.
	//
	// Use a dedicated collision object type (TdEnemy) so you do not pick slots or the floor.

	return nullptr;
}

void UAutoAttackComponent::FireAt(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	// TODO: You write this.
	// Find UHealthComponent on Target and call TakeDamage(Damage).
	// Optionally draw a debug line for the video.
	// Hitscan is enough for Part 1. Projectiles can wait.
}

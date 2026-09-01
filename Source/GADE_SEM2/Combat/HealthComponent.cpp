#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetHealth();
}

void UHealthComponent::ResetHealth()
{
	bDead = false;
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::TakeDamage(float Amount)
{
	if (bDead || Amount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// TODO (UI, Day 7): flash the owner's mesh (emissive or custom depth) so hits are readable.
	// The rubric deducts for missing hit feedback.

	if (CurrentHealth <= 0.f)
	{
		bDead = true;
		OnDeath.Broadcast();
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (bDead || Amount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

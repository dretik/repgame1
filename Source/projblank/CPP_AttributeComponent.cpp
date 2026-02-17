#include "CPP_AttributeComponent.h"

UCPP_AttributeComponent::UCPP_AttributeComponent()
{
	Health = 100.0f;
	MaxHealth = 100.0f;
}

void UCPP_AttributeComponent::InitializeStats(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
	Health = MaxHealth;

	// Ѕроадкастим начальное состо€ние, чтобы UI обновилс€
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(nullptr, this, Health, 0.0f);
	}
}

bool UCPP_AttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}

bool UCPP_AttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	// ≈сли уже мертв и пытаемс€ нанести урон - игнорируем (но лечить трупы можно запретить отдельно)
	if (!IsAlive() && Delta < 0.0f)
	{
		return false;
	}

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);

	float ActualDelta = NewHealth - OldHealth;

	// ≈сли здоровье не изменилось (например, лечение полного HP), выходим
	if (FMath::IsNearlyZero(ActualDelta))
	{
		return false;
	}

	Health = NewHealth;

	// ”ведомл€ем всех слушателей (UI, AI)
	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
	}

	// ѕроверка на смерть
	if (ActualDelta < 0.0f && Health <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		if (OnDeath.IsBound())
		{
			OnDeath.Broadcast(InstigatorActor);
		}
	}

	return true;
}
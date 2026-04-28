#include "CPP_AttributeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCPP_AttributeComponent::UCPP_AttributeComponent()
{
}

void UCPP_AttributeComponent::InitializeStats(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
	Health = MaxHealth;

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
	if (!IsAlive() && Delta < 0.0f)
	{
		return false;
	}

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);

	float ActualDelta = NewHealth - OldHealth;

	if (FMath::IsNearlyZero(ActualDelta))
	{
		return false;
	}

	Health = NewHealth;

	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
	}

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

void UCPP_AttributeComponent::SetBaseSpeed(float NewBase)
{
	BaseSpeed = NewBase;
	UpdateActualMovementSpeed();
}

void UCPP_AttributeComponent::AddSpeedMultiplier(UObject* Source, float Multiplier)
{
	if (Source)
	{
		SpeedMultipliers.Add(Source, Multiplier);
		UpdateActualMovementSpeed();
	}
}

void UCPP_AttributeComponent::RemoveSpeedMultiplier(UObject* Source)
{
	if (Source)
	{
		SpeedMultipliers.Remove(Source);
		UpdateActualMovementSpeed();
	}
}

void UCPP_AttributeComponent::UpdateActualMovementSpeed()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar && OwnerChar->GetCharacterMovement())
	{
		float FinalSpeed = BaseSpeed;
		for (auto& It : SpeedMultipliers)
		{
			FinalSpeed *= It.Value;
		}
		OwnerChar->GetCharacterMovement()->MaxWalkSpeed = FinalSpeed;
	}
}
#include "CPP_Action_Explosion.h"
#include "CPP_BaseCharacter.h"
#include "CPP_CombatStatics.h"
#include "CPP_VisualStatics.h"
#include "CPP_ActionComponent.h"
#include "NiagaraFunctionLibrary.h"

UCPP_Action_Explosion::UCPP_Action_Explosion()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Active.Explosion");
	CooldownTime = 3.0f;

	//default
	MultiplierPerLevel.Add(1.f);
}

void UCPP_Action_Explosion::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	FVector ExplodeLoc = GetActionLocation();

	if (ExplosionVFX)
	{
		UCPP_VisualStatics::SpawnNiagaraEffect(this, ExplosionVFX, ExplodeLoc);
	}

	float BaseAtk = 10.0f;
	if (ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Instigator))
	{
		BaseAtk = BaseChar->GetCurrentBaseDamage();
	}

	float FinalBaseDamage = BaseAtk * GetActionDamageMultiplier();

	UCPP_CombatStatics::ExecuteAreaDamage(
		GetOwningComponent()->GetOwner(), // DamageCauser
		Instigator,                      // Instigator
		ExplodeLoc,
		ExplosionRadius,
		FinalBaseDamage,
		StatusEffects,                   
		PushForce,
		true                             //debug sphere
	);

	StopAction(Instigator);
}

float UCPP_Action_Explosion::GetActionDamageMultiplier() const
{
	UCPP_ActionComponent* Comp = GetOwningComponent();

	int32 CurrentLevel = Comp ? Comp->GetActionLevel(ActionTag) : 1;
	if (CurrentLevel <= 0) CurrentLevel = 1;

	int32 Index = FMath::Clamp(CurrentLevel - 1, 0, MultiplierPerLevel.Num() - 1);

	return (MultiplierPerLevel.Num() > 0) ? MultiplierPerLevel[Index] : 1.0f;
}

FText UCPP_Action_Explosion::GetFormattedDescription_Implementation()
{
	FFormatNamedArguments Args;

	int32 DmgPct = FMath::RoundToInt(GetActionDamageMultiplier() * 100.0f);
	Args.Add("DmgPct", FText::AsNumber(DmgPct));

	Args.Add("Radius", FText::AsNumber(FMath::RoundToInt(ExplosionRadius)));

	// ActionDescription: {Radius}, {DmgPct}% 
	return FText::Format(ActionDescription, Args);
}
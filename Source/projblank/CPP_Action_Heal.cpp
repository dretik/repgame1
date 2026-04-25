#include "CPP_Action_Heal.h"
#include "CPP_AttributeComponent.h"
#include "CPP_BaseCharacter.h"
#include "PaperFlipbookComponent.h"
#include "PaperCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "CPP_CombatStatics.h"

UCPP_Action_Heal::UCPP_Action_Heal()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Boss.Heal");
	Duration = 2.0f;
	HealDelay = 0.5f;
	HealAmount = 30.0f;
}

bool UCPP_Action_Heal::CanStart_Implementation(AActor* Instigator)
{
	if (!Super::CanStart_Implementation(Instigator))
	{
		return false;
	}

	UCPP_AttributeComponent* AttrComp = Instigator->FindComponentByClass<UCPP_AttributeComponent>();
	if (AttrComp)
	{
		if (AttrComp->GetHealth() >= AttrComp->GetMaxHealth())
		{
			return false;
		}
	}

	return true;
}

void UCPP_Action_Heal::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	APaperCharacter* PaperChar = Cast<APaperCharacter>(Instigator);
	if (PaperChar && HealAnim)
	{
		PaperChar->GetSprite()->SetFlipbook(HealAnim);
		PaperChar->GetSprite()->SetLooping(false);
		PaperChar->GetSprite()->PlayFromStart();
	}

	FTimerDelegate Del;
	Del.BindUFunction(this, "ExecuteHeal", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_HealDelay, Del, HealDelay, false);

	if (PaperChar && HealAnim)
	{
		PaperChar->GetSprite()->SetLooping(true);
	}
}

float UCPP_Action_Heal::GetActionDamageMultiplier() const {
	UCPP_ActionComponent* Comp = GetOwningComponent();
	int32 CurrentLevel = Comp ? Comp->GetActionLevel(ActionTag) : 1;
	if (CurrentLevel <= 0) CurrentLevel = 1;

	int32 Index = FMath::Clamp(CurrentLevel - 1, 0, MultiplierPerLevel.Num() - 1);

	return (MultiplierPerLevel.Num() > 0) ? MultiplierPerLevel[Index] : 1.0f;
}

void UCPP_Action_Heal::ExecuteHeal(AActor* Instigator)
{
	if (!Instigator) return;

	float FinalHealValue = 0.0f;
	float Mult = GetActionDamageMultiplier();

	if (HealType == EHealType::Flat)
	{
		float BaseAtk = 10.0f;
		if (ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Instigator))
			BaseAtk = BaseChar->GetCurrentBaseDamage();

		FinalHealValue = BaseAtk * Mult;
	}
	else // Percentage
	{
		// combatstatic waits for 0-100 value
		FinalHealValue = Mult * 100.0f;
	}

		bool bHealed = UCPP_CombatStatics::ExecuteHealing(
			Instigator,
			Instigator,
			FinalHealValue,
			(HealType == EHealType::Percentage)
		);

		if (bHealed)
		{
			if (HealVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HealVFX, Instigator->GetActorLocation());
			}

			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
				FString::Printf(TEXT("Healed: %f, mult: %f (Type: %s)"),
					FinalHealValue, Mult,
					HealType == EHealType::Flat ? TEXT("Flat") : TEXT("Percentage")));
		}
}

FText UCPP_Action_Heal::GetFormattedDescription_Implementation()
{
	FFormatNamedArguments Args;
	int32 DmgPct = FMath::RoundToInt(GetActionDamageMultiplier() * 100.0f);
	Args.Add("DmgPct", FText::AsNumber(DmgPct));

	// Выбираем суффикс в зависимости от типа лечения
	FText Suffix = (HealType == EHealType::Flat)
		? NSLOCTEXT("UI", "HealFlat", "% base ATK")
		: NSLOCTEXT("UI", "HealPct", "% of max HP");

	Args.Add("Suffix", Suffix);

	// В ActionDescription в Блупринте пишем: "Восстанавливает {DmgPct}{Suffix}"
	return FText::Format(ActionDescription, Args);
}
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
}

void UCPP_Action_Heal::ExecuteHeal(AActor* Instigator)
{
	if (!Instigator) return;

		bool bHealed = UCPP_CombatStatics::ExecuteHealing(
			Instigator,
			Instigator,
			HealAmount,
			(HealType == EHealType::Percentage)
		);

		if (bHealed)
		{
			if (HealVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HealVFX, Instigator->GetActorLocation());
			}

			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
				FString::Printf(TEXT("Healed: %f (Type: %s)"),
					HealAmount,
					HealType == EHealType::Flat ? TEXT("Flat") : TEXT("Percentage")));
		}
}
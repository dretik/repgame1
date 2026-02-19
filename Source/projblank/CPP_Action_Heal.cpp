#include "CPP_Action_Heal.h"
#include "CPP_AttributeComponent.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseEnemy.h"
#include "PaperFlipbookComponent.h"
#include "PaperCharacter.h"
#include "NiagaraFunctionLibrary.h"

UCPP_Action_Heal::UCPP_Action_Heal()
{
	// Настраиваем тег и длительность
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Boss.Heal");
	Duration = 2.0f;
	HealDelay = 0.5f;
	HealAmount = 30.0f;
}

bool UCPP_Action_Heal::CanStart_Implementation(AActor* Instigator)
{
	// 1. Стандартные проверки (кулдаун, станы, занятость)
	if (!Super::CanStart_Implementation(Instigator))
	{
		return false;
	}

	UCPP_AttributeComponent* AttrComp = Instigator->FindComponentByClass<UCPP_AttributeComponent>();
	if (AttrComp)
	{
		// Если здоровья и так максимум - запрещаем действие
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

	// 1. Анимация
	APaperCharacter* PaperChar = Cast<APaperCharacter>(Instigator);
	if (PaperChar && HealAnim)
	{
		PaperChar->GetSprite()->SetFlipbook(HealAnim);
		PaperChar->GetSprite()->SetLooping(false);
		PaperChar->GetSprite()->PlayFromStart();
	}

	// 2. Таймер применения эффекта
	FTimerDelegate Del;
	Del.BindUFunction(this, "ExecuteHeal", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_HealDelay, Del, HealDelay, false);
}

void UCPP_Action_Heal::ExecuteHeal(AActor* Instigator)
{
	if (!Instigator) return;

	UCPP_AttributeComponent* AttrComp = Instigator->FindComponentByClass<UCPP_AttributeComponent>();
	ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(Instigator);
	if (AttrComp)
	{
		float FinalHealAmount = 0.f;

		if (HealType == EHealType::Percentage)
		{
			const float PercentageFactor = 100.0f;
			FinalHealAmount = Enemy->GetCurrentMaxHealth() * (HealAmount / PercentageFactor);
		}
		else
		{
			FinalHealAmount = HealAmount;
		}

		if (bApplyLevelScaling && Enemy)
		{
			FinalHealAmount *= Enemy->GetEnemyDamageMultiplier();
		}

		bool bHealed = AttrComp->ApplyHealthChange(Instigator, FinalHealAmount);

		if (bHealed)
		{
			// Визуальный эффект
			if (HealVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HealVFX, Instigator->GetActorLocation());
			}

			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
				FString::Printf(TEXT("Healed: %f (Type: %s)"),
					FinalHealAmount,
					HealType == EHealType::Flat ? TEXT("Flat") : TEXT("Percentage")));
		}
	}
}
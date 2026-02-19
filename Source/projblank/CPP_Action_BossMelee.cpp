// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Action_BossMelee.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseEnemy.h"
#include "PaperFlipbookComponent.h"

UCPP_Action_BossMelee::UCPP_Action_BossMelee()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Boss.Melee");
	CooldownTime = 2.0f;
	Duration = 0.8f;
}

void UCPP_Action_BossMelee::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(Instigator);
	if (Char)
	{
		// 1. Анимация
		if (AttackAnim)
		{
			Char->GetSprite()->SetFlipbook(AttackAnim);
			Char->GetSprite()->SetLooping(false);
			Char->GetSprite()->PlayFromStart();
		}

		// 2. Таймер удара (через 0.4 сек)
		FTimerDelegate HitDelegate;
		HitDelegate.BindUFunction(this, "MakeHit", Instigator);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, HitDelegate, AttackDelay, false);
	
		if (AttackAnim)
		{
			Char->GetSprite()->SetLooping(true);
		}
	}
}

void UCPP_Action_BossMelee::MakeHit(AActor* Instigator)
{
	ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(Instigator);
	if (!Char) return;
	ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(Instigator);
	UCharacterStats* Stats = Char->GetCharacterStats();
	if (!Stats) return;
	float DamageToApply = Stats->LightAttackDamage;

	if (Enemy)
	{
		DamageToApply *= Enemy->GetEnemyDamageMultiplier();
	}
		
	Char->PerformAttackTrace(
		Stats->LightAttackRange, 
		Stats->LightAttackBoxSize, 
		DamageToApply);
}

void UCPP_Action_BossMelee::StopAction_Implementation(AActor* Instigator)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("TAG REMOVED: %s at time %f"), *ActionTag.ToString(), GetWorld()->GetTimeSeconds()));
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AttackDelay);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Duration);

	Super::StopAction_Implementation(Instigator);
}
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
		if (AttackAnim)
		{
			Char->GetSprite()->SetFlipbook(AttackAnim);
			Char->GetSprite()->SetLooping(false);
			Char->GetSprite()->PlayFromStart();
		}

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
	UCharacterStats* Stats = Char->GetCharacterStats();
	if (!Stats) return;
	float DamageToApply = 0.f;

	if (Char->IsPlayerControlled())
	{
		DamageToApply = Char->GetCurrentBaseDamage();
	}
	else if (ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(Char))
	{
		if (Stats)
		{
			// lightattackdamage as base but could be replaced as action variable
			DamageToApply = Stats->LightAttackDamage * Enemy->GetEnemyDamageMultiplier();
		}
	}
	DamageToApply *= ActionDamageMultiplier;
		
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
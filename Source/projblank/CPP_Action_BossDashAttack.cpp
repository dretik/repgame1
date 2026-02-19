// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Action_BossDashAttack.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseEnemy.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UCPP_Action_BossDashAttack::UCPP_Action_BossDashAttack()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Boss.DashAttack");
	Duration = 1.0f;
}

void UCPP_Action_BossDashAttack::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(Instigator);
	if (!Char) return;
	UCharacterStats* Stats = Char->GetCharacterStats();
	if (!Stats) return;
		// 1. Анимация
		if (DashAnim)
		{
			Char->GetSprite()->SetFlipbook(DashAnim);
			Char->GetSprite()->SetLooping(false);
			Char->GetSprite()->PlayFromStart();
		}

		// 2. Логика Рывка (Launch Character)
		FVector DashDir = FVector(0.0f, 1.0f, 0.0f); // Вправо по Y

		// Проверяем направление спрайта
		if (Char->GetSprite()->GetRelativeScale3D().X < 0.0f)
		{
			DashDir *= -1.0f; // Влево
		}

		Char->LaunchCharacter(DashDir * Stats->DashAttackImpulse, true, true);

		// 3. Таймер удара в конце рывка
		FTimerDelegate HitDelegate;
		HitDelegate.BindUFunction(this, "MakeDashHit", Instigator);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Hit, HitDelegate, HitDelay, false);
	
		if (DashAnim)
		{
			Char->GetSprite()->SetLooping(true);

		}
}

void UCPP_Action_BossDashAttack::MakeDashHit(AActor* Instigator)
{
	ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(Instigator);
	if (!Char) return;
	ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(Instigator);
	UCharacterStats* Stats = Char->GetCharacterStats();
	if (!Stats) return;
	float DamageToApply = Stats->DashAttackDamage;

	if (Enemy)
	{
		DamageToApply *= Enemy->GetEnemyDamageMultiplier();
	}
	Char->PerformAttackTrace(
		Stats->DashAttackRange,
		Stats->DashAttackBoxSize,
		DamageToApply);
}

void UCPP_Action_BossDashAttack::StopAction_Implementation(AActor* Instigator)
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		//FString::Printf(TEXT("TAG REMOVED: %s at time %f"), *ActionTag.ToString(), GetWorld()->GetTimeSeconds()));
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Hit);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Duration);
	Super::StopAction_Implementation(Instigator);
}
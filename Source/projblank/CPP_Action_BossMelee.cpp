// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Action_BossMelee.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseEnemy.h"
#include "CPP_AttributeComponent.h"
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
	if (!Char) return;

	if (AttackAnim)
	{
		Char->GetSprite()->SetFlipbook(AttackAnim);
		Char->GetSprite()->SetLooping(false);
		Char->GetSprite()->PlayFromStart();
	}

	if (DashImpulse > 0.0f)
	{
		FVector ActualDashDir = DashDirection;

		if (Char->GetSprite()->GetRelativeScale3D().X < 0.0f)
		{
			ActualDashDir.X *= -1.0f;
			ActualDashDir.Y *= -1.0f;
		}

		ActualDashDir.Normalize();

		Char->LaunchCharacter(ActualDashDir * DashImpulse, true, true);
	}

	if (WalkSpeedModifier >= 0.0f && Char->GetCharacterMovement())
	{
		OriginalWalkSpeed = Char->GetCharacterMovement()->MaxWalkSpeed;
		Char->GetCharacterMovement()->MaxWalkSpeed = WalkSpeedModifier;
	}

	FTimerDelegate HitDelegate;
	HitDelegate.BindUFunction(this, "MakeHit", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, HitDelegate, AttackDelay, false);
	
	if (AttackAnim)
	{
		Char->GetSprite()->SetLooping(true);
	}	
}

void UCPP_Action_BossMelee::MakeHit(AActor* Instigator)
{
	ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(Instigator);
	if (!Char) return;
	float DamageToApply = Char->GetCurrentBaseDamage();

	if (UCPP_AttributeComponent* AttrComp = Char->FindComponentByClass<UCPP_AttributeComponent>())
	{
		DamageToApply *= AttrComp->GetDamageMultiplier();
	}

	DamageToApply *= ActionDamageMultiplier;
		
	Char->PerformAttackTrace(
		AttackRange,
		AttackBoxSize,
		DamageToApply);
}

void UCPP_Action_BossMelee::StopAction_Implementation(AActor* Instigator)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("TAG REMOVED: %s at time %f"), *ActionTag.ToString(), GetWorld()->GetTimeSeconds()));
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AttackDelay);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Duration);
	ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(Instigator);
	if (Char)
	{
		if (WalkSpeedModifier >= 0.0f && OriginalWalkSpeed >= 0.0f)
		{
			Char->GetCharacterMovement()->MaxWalkSpeed = OriginalWalkSpeed;
		}
	}
	Super::StopAction_Implementation(Instigator);
}
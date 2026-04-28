#include "CPP_Action_Projectile.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Action_Effect.h"
#include "CPP_Projectile.h"
#include "PaperFlipbookComponent.h" 
#include "PaperCharacter.h" 
#include "CPP_BaseCharacter.h" 
#include "CPP_AttributeComponent.h"
#include "CPP_VisualComponent.h"
#include "CPP_CombatStatics.h"

UCPP_Action_Projectile::UCPP_Action_Projectile()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Active.Fireball");
	CooldownTime = 2.0f;
	AttackAnimDelay = 0.2f;
}

void UCPP_Action_Projectile::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	ACharacter* Character = Cast<ACharacter>(Instigator);
	if (Character)
	{
		APaperCharacter* PaperChar = Cast<APaperCharacter>(Character);
		if (PaperChar && CastAnim)
		{
			PaperChar->GetSprite()->SetFlipbook(CastAnim);
		}

		FTimerHandle TimerHandle_AttackDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, AttackAnimDelay, false);
	}
}

void UCPP_Action_Projectile::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	if (!InstigatorCharacter)
	{
		StopAction(InstigatorCharacter);
		return;
	}

	UCPP_ActionComponent* Comp = GetOwningComponent();
	if (!Comp) { StopAction(InstigatorCharacter); return; }

	int32 CurrentLevel = Comp->GetActionLevel(ActionTag);

	int32 ConfigIndex = FMath::Clamp(CurrentLevel - 1, 0, LevelConfigs.Num() - 1);

	if (LevelConfigs.Num() > 0)
	{
		const FProjectileLevelData& Cfg = LevelConfigs[ConfigIndex];

		UCPP_CombatStatics::SpawnProjectile(
			InstigatorCharacter,
			Cfg.ProjectileClass,
			GetCurrentDamage(),
			Cfg.FlightSpeed,
			Cfg.ExplosionRadius,
			Cfg.ProjectileScale,
			Cfg.ImpactEffects
		);
	}

	StopAction(InstigatorCharacter);
}

float UCPP_Action_Projectile::GetActionDamageMultiplier() const
{
	UCPP_ActionComponent* Comp = GetOwningComponent();

	// for CDO (preview): if no comp (template), take level 1.
	int32 CurrentLevel = Comp ? Comp->GetActionLevel(ActionTag) : 1;
	// 0 level protection
	if (CurrentLevel <= 0) CurrentLevel = 1;

	int32 ConfigIndex = FMath::Clamp(CurrentLevel - 1, 0, LevelConfigs.Num() - 1);

	if (LevelConfigs.Num() == 0) return 1.0f;

	return LevelConfigs[ConfigIndex].DamageMultiplier;
}

float UCPP_Action_Projectile::GetCurrentDamage() const
{
	UCPP_ActionComponent* Comp = GetOwningComponent();
	if (!Comp) return 0.0f;

	float BaseDmg = 10.0f; // fallback
	if (ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Comp->GetOwner()))
	{
		BaseDmg = BaseChar->GetCurrentBaseDamage();
	}

	return BaseDmg * GetActionDamageMultiplier();
}
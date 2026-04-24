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

	if (LevelConfigs.Num() == 0)
	{
		StopAction(InstigatorCharacter);
		return;
	}

	const FProjectileLevelData& CurrentConfig = LevelConfigs[ConfigIndex];

	UCPP_VisualComponent* VisualComp = InstigatorCharacter->FindComponentByClass<UCPP_VisualComponent>();
	FVector FacingDir = VisualComp ? VisualComp->GetVisualFacingDirection() : FVector(0, 1, 0);

	float BaseOffset = 40.0f;
	float FinalOffset = BaseOffset * CurrentConfig.ProjectileScale;

	FRotator SpawnRotation = FRotator(0, (FacingDir.Y > 0 ? 90.f : -90.f), 0);
	FVector SpawnLocation = InstigatorCharacter->GetActorLocation();
	SpawnLocation.Y += FacingDir.Y * FinalOffset;
	SpawnLocation.Z += 20.0f;

	FActorSpawnParameters SpawnParams;
	//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = InstigatorCharacter;
	SpawnParams.Owner = InstigatorCharacter;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(CurrentConfig.ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedActor)
	{
		InstigatorCharacter->MoveIgnoreActorAdd(SpawnedActor);
	}

	if (ACPP_Projectile* Proj = Cast<ACPP_Projectile>(SpawnedActor))
	{
		Proj->SetDamage(GetCurrentDamage());
        Proj->SetActorScale3D(FVector(CurrentConfig.ProjectileScale));
		Proj->SetPersistentEffects(CurrentConfig.ImpactEffects);
	}

	StopAction(InstigatorCharacter);
}

FText UCPP_Action_Projectile::GetFormattedDescription_Implementation()
{
	// could take ActionDescription and replace {Dmg} with actual data
	// using FText::Format
	FFormatNamedArguments Args;
	Args.Add("Dmg", FText::AsNumber(FMath::RoundToInt(GetCurrentDamage())));

	return FText::Format(ActionDescription, Args);
}

float UCPP_Action_Projectile::GetCurrentDamage() const
{
	UCPP_ActionComponent* Comp = GetOwningComponent();
	if (!Comp) return 0.0f;

	//getting lvl
	int32 CurrentLevel = Comp->GetActionLevel(ActionTag);
	int32 ConfigIndex = FMath::Clamp(CurrentLevel - 1, 0, LevelConfigs.Num() - 1);

	if (LevelConfigs.Num() == 0) return 0.0f;

	const FProjectileLevelData& Config = LevelConfigs[ConfigIndex];

	// owners base dmg
	float BaseDmg = 0.0f;
	if (ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Comp->GetOwner()))
	{
		BaseDmg = BaseChar->GetCurrentBaseDamage();
	}
	else
	{
		// if CDO (shop preview), taking damage from first cfg
		return Config.DamageMultiplier * 10.0f; // default
	}

	return BaseDmg * Config.DamageMultiplier;
}
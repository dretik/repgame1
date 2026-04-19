#include "CPP_Action_Projectile.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Projectile.h"
#include "PaperFlipbookComponent.h" 
#include "PaperCharacter.h" 
#include "CPP_BaseCharacter.h" 
#include "CPP_PlayerCharacter.h"
#include "CPP_AttributeComponent.h" 

UCPP_Action_Projectile::UCPP_Action_Projectile()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Player.Range.Fireball");
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

	FVector SpawnLocation = InstigatorCharacter->GetActorLocation();
	SpawnLocation.Z += 20.0f;

	FRotator SpawnRotation = FRotator::ZeroRotator;

	float BaseOffset = 40.0f;
	float FinalOffset = BaseOffset * CurrentConfig.ProjectileScale;

	APaperCharacter* PaperChar = Cast<APaperCharacter>(InstigatorCharacter);
	if (PaperChar)
	{
		if (PaperChar->GetSprite()->GetRelativeScale3D().X > 0.0f)
		{
			SpawnRotation.Yaw = 90.0f;
			SpawnLocation.Y += FinalOffset;
		}
		else
		{
			SpawnRotation.Yaw = -90.0f;
			SpawnLocation.Y -= FinalOffset;
		}
	}

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
		float TotalDmg = 0.f;
		if (ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(InstigatorCharacter)) {
			TotalDmg = BaseChar->GetCurrentBaseDamage() * CurrentConfig.DamageMultiplier;
		}

		Proj->SetDamage(TotalDmg);
        Proj->SetActorScale3D(FVector(CurrentConfig.ProjectileScale));
		Proj->SetPersistentEffects(CurrentConfig.ImpactEffects);
	}

	StopAction(InstigatorCharacter);
}
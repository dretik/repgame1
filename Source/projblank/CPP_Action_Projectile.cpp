#include "CPP_Action_Projectile.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Projectile.h"
#include "PaperFlipbookComponent.h" 
#include "PaperCharacter.h" 

// Для доступа к статам персонажа (урон), если нужно
#include "CPP_BaseCharacter.h" 
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
		// Проигрываем анимацию, если есть (через PaperCharacter)
		APaperCharacter* PaperChar = Cast<APaperCharacter>(Character);
		if (PaperChar && CastAnim)
		{
			PaperChar->GetSprite()->SetFlipbook(CastAnim);
			// В идеале: вернуть Idle анимацию после окончания, но это уже детали анимации
		}

		// Запускаем таймер на спавн (чтобы снаряд вылетел не мгновенно, а в нужный кадр анимации)
		FTimerHandle TimerHandle_AttackDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, AttackAnimDelay, false);
	}
}

void UCPP_Action_Projectile::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	if (!ProjectileClass || !InstigatorCharacter)
	{
		StopAction(InstigatorCharacter);
		return;
	}

	ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(InstigatorCharacter);
	if (!BaseChar) return;

	int32 CurrentLevel = BaseChar->GetAbilityLevel(ActionTag);

	if (CurrentLevel <= 0) CurrentLevel = 1;

	FVector SpawnLocation = InstigatorCharacter->GetActorLocation();
	SpawnLocation.Z += 20.0f;

	// Определяем направление по скейлу спрайта (как у тебя в BaseCharacter)
	FRotator SpawnRotation = FRotator::ZeroRotator;
	APaperCharacter* PaperChar = Cast<APaperCharacter>(InstigatorCharacter);
	if (PaperChar)
	{
		if (PaperChar->GetSprite()->GetRelativeScale3D().X > 0.0f)
		{
			SpawnRotation.Yaw = 90.0f;
			SpawnLocation.Y += 40.0f;
		}
		else
		{
			SpawnRotation.Yaw = -90.0f;
			SpawnLocation.Y -= 40.0f;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = InstigatorCharacter;
	SpawnParams.Owner = InstigatorCharacter;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	// Настройка урона (берем из статов персонажа)
	ACPP_Projectile* Proj = Cast<ACPP_Projectile>(SpawnedActor);

	if (Proj && BaseChar)
	{
		// Используем твой геттер базового урона
		float Dmg = BaseChar->GetCurrentBaseDamage();
		Proj->SetDamage(Dmg);
	}

	StopAction(InstigatorCharacter);
}
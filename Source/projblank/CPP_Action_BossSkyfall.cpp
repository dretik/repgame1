#include "CPP_Action_BossSkyfall.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Projectile.h"
#include "CPP_BaseEnemy.h"
#include "PaperFlipbookComponent.h"
#include "PaperCharacter.h"
#include "PaperFlipbook.h"
#include "GameFramework/CharacterMovementComponent.h" 

UCPP_Action_BossSkyfall::UCPP_Action_BossSkyfall()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Boss.Skyfall");
	Duration = 5.0f;
}

void UCPP_Action_BossSkyfall::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	MeteorsSpawned = 0;

	// Останавливаем босса
	ACharacter* Char = Cast<ACharacter>(Instigator);
	if (!Char) return;
	UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();

	if (MoveComp)
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_None);
	}

	APaperCharacter* PaperChar = Cast<APaperCharacter>(Char);
	float IntroDuration = 0.1f; // Фоллбэк

	if (PaperChar && IntroAnim)
	{
		PaperChar->GetSprite()->SetFlipbook(IntroAnim);
		PaperChar->GetSprite()->SetLooping(false);
		PaperChar->GetSprite()->PlayFromStart();

		IntroDuration = IntroAnim->GetTotalDuration();
	}

	// 3. ТАЙМЕР НА ПЕРЕКЛЮЧЕНИЕ В ЦИКЛ (Loop)
	FTimerHandle TimerHandle_Intro;
	FTimerDelegate IntroDelegate;
	IntroDelegate.BindUFunction(this, "SwitchToLoopAnim", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Intro, IntroDelegate, IntroDuration, false);
}

void UCPP_Action_BossSkyfall::SwitchToLoopAnim(AActor* Instigator)
{
	if (!IsRunning()) return; // Если действие уже прервано, ничего не делаем

	APaperCharacter* PaperChar = Cast<APaperCharacter>(Instigator);
	if (PaperChar && LoopAnim)
	{
		PaperChar->GetSprite()->SetFlipbook(LoopAnim);
		PaperChar->GetSprite()->SetLooping(true);
		PaperChar->GetSprite()->Play();
	}

	// ТОЛЬКО ТЕПЕРЬ ЗАПУСКАЕМ СПАВН МЕТЕОРОВ
	MeteorsSpawned = 0;
	FTimerDelegate SpawnDel;
	SpawnDel.BindUFunction(this, "SpawnMeteor", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Spawn, SpawnDel, SpawnRate, true);
}

void UCPP_Action_BossSkyfall::StopAction_Implementation(AActor* Instigator)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("TAG REMOVED: %s at time %f"), *ActionTag.ToString(), GetWorld()->GetTimeSeconds()));
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Spawn);

	// Возвращаем движение
	ACharacter* Char = Cast<ACharacter>(Instigator);
	if (Char)
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	Super::StopAction_Implementation(Instigator);
}

void UCPP_Action_BossSkyfall::SpawnMeteor(AActor* Instigator)
{
	if (!GetWorld() || !ProjectileClass)
	{
		StopAction(Instigator);
		return;
	}
	
	if (MeteorsSpawned >= MeteorsCount)
	{
		StopAction(Instigator); // Сами останавливаем действие
		return;
	}

	MeteorsSpawned++;

	FVector TargetLocation = FVector::ZeroVector;
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (PlayerPawn)
	{
		TargetLocation = PlayerPawn->GetActorLocation();
	}
	else
	{
		// Fallback: Если игрока нет (умер), спавним вокруг Босса (Instigator), чтобы не было ошибок
		if (Instigator)
		{
			TargetLocation = Instigator->GetActorLocation();
		}
	}

	// 4. Расчет позиции спавна (2.5D логика)
	FVector SpawnLocation = TargetLocation;
	SpawnLocation.Z += SpawnHeight; // Поднимаем вверх

	// Рандомный разброс (как в твоем старом коде)
	SpawnLocation.X += FMath::RandRange(-SpawnRadius, SpawnRadius);
	SpawnLocation.Y += FMath::RandRange(-SpawnRadius, SpawnRadius);

	// 5. Ротация (Вниз)
	FRotator SpawnRotation = FRotator(-90.0f, 0.0f, 0.0f);

	// 6. Параметры спавна
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = Instigator;           // Владелец снаряда - Босс
	Params.Instigator = Cast<APawn>(Instigator); // Инстигатор урона - Босс

	// 7. Спавн
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

	// 8. Настройка снаряда
	if (SpawnedActor)
	{
		ACPP_Projectile* Meteor = Cast<ACPP_Projectile>(SpawnedActor);
		ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(Instigator);
		if (Meteor&&Enemy)
		{
			float DamageToApply = DamagePerMeteor * Enemy->GetEnemyDamageMultiplier();
			Meteor->SetDamage(DamageToApply);
		}

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Purple, TEXT("Meteor Spawned via Action!"));
	}
}
#include "CPP_Action_BossSkyfall.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Projectile.h"
#include "PaperFlipbookComponent.h"
#include "PaperCharacter.h"
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
	if (Char)
	{
		Char->GetCharacterMovement()->StopMovementImmediately();

		// Меняем анимацию (если нужно)
		APaperCharacter* PaperChar = Cast<APaperCharacter>(Char);
		if (PaperChar && LoopAnim)
		{
			PaperChar->GetSprite()->SetFlipbook(LoopAnim);
		}
	}

	// Запускаем таймер спавна
	FTimerDelegate Del;
	Del.BindUFunction(this, "SpawnMeteor", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Spawn, Del, SpawnRate, true);
}

void UCPP_Action_BossSkyfall::StopAction_Implementation(AActor* Instigator)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Spawn);

	// Возвращаем движение
	ACharacter* Char = Cast<ACharacter>(Instigator);
	if (Char)
	{
		// Вернуть MaxWalkSpeed или просто разблокировать движение
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
		if (Meteor)
		{
			// Передаем урон, настроенный внутри Action (Data-Driven)
			Meteor->SetDamage(DamagePerMeteor);
		}

		// Лог для отладки (можно убрать на релизе)
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Purple, TEXT("Meteor Spawned via Action!"));
	}
}
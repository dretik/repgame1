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

	ACharacter* Char = Cast<ACharacter>(Instigator);
	if (!Char) return;
	UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();

	if (MoveComp)
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_None);
	}

	APaperCharacter* PaperChar = Cast<APaperCharacter>(Char);
	float IntroDuration = 0.1f; // fallback

	if (PaperChar && IntroAnim)
	{
		PaperChar->GetSprite()->SetFlipbook(IntroAnim);
		PaperChar->GetSprite()->SetLooping(false);
		PaperChar->GetSprite()->PlayFromStart();

		IntroDuration = IntroAnim->GetTotalDuration();
	}

	FTimerHandle TimerHandle_Intro;
	FTimerDelegate IntroDelegate;
	IntroDelegate.BindUFunction(this, "SwitchToLoopAnim", Instigator);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Intro, IntroDelegate, IntroDuration, false);
}

void UCPP_Action_BossSkyfall::SwitchToLoopAnim(AActor* Instigator)
{
	if (!IsRunning()) return;

	APaperCharacter* PaperChar = Cast<APaperCharacter>(Instigator);
	if (PaperChar && LoopAnim)
	{
		PaperChar->GetSprite()->SetFlipbook(LoopAnim);
		PaperChar->GetSprite()->SetLooping(true);
		PaperChar->GetSprite()->Play();
	}

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
		StopAction(Instigator);
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
		if (Instigator)
		{
			TargetLocation = Instigator->GetActorLocation();
		}
	}

	FVector SpawnLocation = TargetLocation;
	SpawnLocation.Z += SpawnHeight; 

	SpawnLocation.X += FMath::RandRange(-SpawnRadius, SpawnRadius);
	SpawnLocation.Y += FMath::RandRange(-SpawnRadius, SpawnRadius);

	FRotator SpawnRotation = FRotator(-90.0f, 0.0f, 0.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = Instigator;
	Params.Instigator = Cast<APawn>(Instigator); 

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

	if (SpawnedActor)
	{
		ACPP_Projectile* Meteor = Cast<ACPP_Projectile>(SpawnedActor);
		if (Meteor)
		{

			Meteor->SetDamage(DamagePerMeteor);
		}

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Purple, TEXT("Meteor Spawned via Action!"));
	}
}
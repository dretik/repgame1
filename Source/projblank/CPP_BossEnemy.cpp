// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BossEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Projectile.h"
#include "CharacterStats.h"

ACPP_BossEnemy::ACPP_BossEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GetSprite()->SetRelativeScale3D(FVector(2.5f));
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;

    // Увеличим капсулу, если нужно
    // GetCapsuleComponent()->SetCapsuleSize(...);
}

void ACPP_BossEnemy::AttackPlayer()
{
    if (bIsAttacking) return;

    float TotalWeight = 0.0f;
    for (const FBossAttackConfig& Config : AttackConfigs)
    {
        TotalWeight += Config.Weight;
    }

    if (TotalWeight <= 0.0f) return;

    float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;

    for (const FBossAttackConfig& Config : AttackConfigs)
    {
        CurrentSum += Config.Weight;
        if (RandomPoint <= CurrentSum)
        {
            ExecuteAttack(Config);
            return;
        }
    }
}

void ACPP_BossEnemy::ExecuteAttack(const FBossAttackConfig& Config)
{
    bIsAttacking = true;

    if (Config.AttackAnim)
    {
        GetSprite()->SetFlipbook(Config.AttackAnim);
        GetSprite()->SetLooping(false);
        GetSprite()->PlayFromStart();
    }

    float ScaledDamage = Config.Damage * EnemyLevelDamageMultiplier;

    switch (Config.AttackType)
    {
    case EBossAttackType::Melee:
    {
        FTimerHandle HitTimer;
        GetWorldTimerManager().SetTimer(HitTimer, [this, ScaledDamage]() {
            DoMeleeAttack(ScaledDamage);
            }, 0.4f, false);

        FTimerHandle FinishTimer;
        GetWorldTimerManager().SetTimer(FinishTimer, this, &ACPP_BossEnemy::FinishAttack, Config.Duration, false);
    }
    break;

    case EBossAttackType::Dash:
    {
        DoDashAttack(ScaledDamage);
        FTimerHandle FinishTimer;
        GetWorldTimerManager().SetTimer(FinishTimer, this, &ACPP_BossEnemy::FinishAttack, Config.Duration, false);
    }
    break;

    case EBossAttackType::Skyfall:
    {
        CurrentSkyfallDamage = ScaledDamage;

        CurrentLoopAnim = Config.AttackAnimLoop;

        float IntroDuration = 0.5f;
        if (Config.AttackAnim)
        {
            IntroDuration = Config.AttackAnim->GetTotalDuration();
        }

        float LoopDuration = FMath::Max(0.1f, Config.Duration - IntroDuration);

        FTimerDelegate IntroDel;
        IntroDel.BindUFunction(this, FName("OnSkyfallIntroFinished"), LoopDuration);

        FTimerHandle IntroTimer;
        GetWorldTimerManager().SetTimer(IntroTimer, IntroDel, IntroDuration, false);
    }
    break;
    }
}

void ACPP_BossEnemy::OnSkyfallIntroFinished(float LoopDuration)
{
    if (CurrentLoopAnim)
    {
        GetSprite()->SetFlipbook(CurrentLoopAnim);
        GetSprite()->SetLooping(true);
        GetSprite()->PlayFromStart();
    }

    StartSkyfall();

    FTimerHandle FinishTimer;
    GetWorldTimerManager().SetTimer(FinishTimer, this, &ACPP_BossEnemy::FinishAttack, LoopDuration, false);
}

void ACPP_BossEnemy::DoMeleeAttack(float Damage)
{
    PerformAttackTrace(
        200.0f,
        FVector(200.f, 80.f, 80.f),
        Damage
    );
}

void ACPP_BossEnemy::DoDashAttack(float Damage)
{
    FVector DashDir = FVector(0.0f, 1.0f, 0.0f);

    if (GetSprite()->GetRelativeScale3D().X < 0.0f)
    {
        DashDir *= -1.0f;
    }

    LaunchCharacter(DashDir * 2000.0f, true, true);

    FTimerHandle HitTimer;
    GetWorldTimerManager().SetTimer(HitTimer, [this, ConfigDamage = Damage]() {
        DoMeleeAttack(ConfigDamage);
        }, 0.3f, false);
}

void ACPP_BossEnemy::StartSkyfall()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    MeteorsToSpawn = 80;
    GetWorldTimerManager().SetTimer(SkyfallTimer, this, &ACPP_BossEnemy::SpawnSingleMeteor, 0.05f, true);
}

void ACPP_BossEnemy::SpawnSingleMeteor()
{
    if (MeteorsToSpawn <= 0)
    {
        GetWorldTimerManager().ClearTimer(SkyfallTimer);
        return;
    }
    MeteorsToSpawn--;

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, TEXT("Trying to spawn Meteor..."));

    if (!SkyfallProjectileClass)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ERROR: SkyfallProjectileClass is NONE!"));
        return;
    }

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (Player && SkyfallProjectileClass)
    {
        FVector PlayerLoc = Player->GetActorLocation();

        FVector SpawnLoc = PlayerLoc;
        SpawnLoc.Z += 500.0f;
        SpawnLoc.X += FMath::RandRange(-200.f, 200.f);
        SpawnLoc.Y += FMath::RandRange(-200.f, 200.f);

        FRotator SpawnRot = FRotator(-90.0f, 0.0f, 0.0f);

        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = GetInstigator();

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SkyfallProjectileClass, SpawnLoc, SpawnRot, Params);
        
        ACPP_Projectile* Meteor = Cast<ACPP_Projectile>(SpawnedActor);
        if (Meteor)
        {
            Meteor->SetDamage(CurrentSkyfallDamage);
        }

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Meteor Spawned!"));
    }
}

void ACPP_BossEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsAttacking) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (Player)
    {
        float ZDiff = Player->GetActorLocation().Z - GetActorLocation().Z;

        if (ZDiff > 150.0f && !GetCharacterMovement()->IsFalling())
        {
            Jump();

            FVector JumpDir = Player->GetActorLocation() - GetActorLocation();
            JumpDir.Z = 0;
            JumpDir.Normalize();
            GetCharacterMovement()->Velocity += JumpDir * 300.0f;
        }
    }
}

void ACPP_BossEnemy::FinishAttack()
{
    GetCharacterMovement()->MaxWalkSpeed = CharacterStats->MaxWalkSpeed;

    Super::FinishAttack();
}
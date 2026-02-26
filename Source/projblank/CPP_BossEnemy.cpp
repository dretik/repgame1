// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BossEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Projectile.h"
#include "CharacterStats.h"
#include "CPP_ActionComponent.h"

ACPP_BossEnemy::ACPP_BossEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GetSprite()->SetRelativeScale3D(FVector(2.5f));
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;

    GetSprite()->SetRelativeScale3D(FVector(2.5f));
}

void ACPP_BossEnemy::AttackPlayer()
{
    if (AttackConfigs.Num() == 0) return;

    if (IsDead() || GetIsAttacking()) return;

    if (!ActionComp) return;

    float TotalWeight = 0.0f;
    for (const FBossAttackConfig& Config : AttackConfigs)
    {
        TotalWeight += Config.Weight;
    }

    if (TotalWeight <= 0.0f) return;

    float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;
    FGameplayTag SelectedTag;

    for (const FBossAttackConfig& Config : AttackConfigs)
    {
        CurrentSum += Config.Weight;
        if (RandomPoint <= CurrentSum)
        {
            SelectedTag = Config.ActionTag;
            break;
        }
    }

    if (SelectedTag.IsValid())
    {
        bool bSuccess = ActionComp->StartActionByName(this, SelectedTag);

        if (bSuccess)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
                FString::Printf(TEXT("Boss starting attack: %s"), *SelectedTag.ToString()));
        }
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

// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_BaseEnemy.h"
#include "Perception/PawnSensingComponent.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "CPP_BaseItem.h"
#include "CharacterStats.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_Item_Currency.h"
#include "CPP_Item_XP.h"

ACPP_BaseEnemy::ACPP_BaseEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

    PawnSensingComp->SetPeripheralVisionAngle(60.0f);
    PawnSensingComp->SightRadius = 800.0f;
    PawnSensingComp->HearingThreshold = 600.0f;
    PawnSensingComp->LOSHearingThreshold = 1200.0f;


    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
    }
}

void ACPP_BaseEnemy::BeginPlay()
{
    Super::BeginPlay();

    ACPP_BaseCharacter* Player = Cast<ACPP_BaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

    if (PawnSensingComp)
    {
        PawnSensingComp->OnSeePawn.AddDynamic(this, &ACPP_BaseEnemy::OnPawnSeen);
    }
    if (Player && CharacterStats)
    {
        int32 PlayerLevel = Player->GetCharacterLevel();

        if (PlayerLevel > 1)
        {
            float HealthScale = 1.0f + ((PlayerLevel - 1) * CharacterStats->HealthScalingFactor);
            float NewMaxHealth = CharacterStats->MaxHealth * HealthScale;

            CurrentMaxHealth = NewMaxHealth; 
            CurrentHealth = NewMaxHealth;    

            EnemyLevelDamageMultiplier = 1.0f + ((PlayerLevel - 1) * CharacterStats->DamageScalingFactor);

        }
    }
    OnHealthChanged.Broadcast(CurrentHealth, CurrentMaxHealth);
}

void ACPP_BaseEnemy::OnPawnSeen(APawn* SeenPawn)
{
    if (SeenPawn == nullptr) return;

    AAIController* AIController = Cast<AAIController>(GetController());

    if (AIController && AIController->GetBlackboardComponent())
    {
        AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SeenPawn);

        AIController->GetBlackboardComponent()->SetValueAsFloat("StoppingDistance", StoppingDistance);
    }
}

void ACPP_BaseEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const FVector Velocity = GetVelocity();

    if (!Velocity.IsNearlyZero())
    {
        FVector CurrentScale = GetSprite()->GetRelativeScale3D();

        if (Velocity.Y > 0.1f)
        {
            CurrentScale.X = FMath::Abs(CurrentScale.X); 
        }
        else if (Velocity.Y < -0.1f)
        {
            CurrentScale.X = -FMath::Abs(CurrentScale.X); 
        }

        GetSprite()->SetRelativeScale3D(CurrentScale);
    }
}

void ACPP_BaseEnemy::AttackPlayer()
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("AI Trying to Attack..."));
    if (!CharacterStats) {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("FAIL: No Stats!"));
        return;
    }
    if (bIsAttacking) { 
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("FAIL: Already Attacking!"));
        return; 
    }

    bIsAttacking = true;

    float AttackDuration = 0.5f;

    if (Attack1Flipbook) GetSprite()->SetFlipbook(Attack1Flipbook);
    AttackDuration = CharacterStats->Attack1Duration;

    FTimerHandle HitTimer;
    GetWorldTimerManager().SetTimer(HitTimer, this, &ACPP_BaseEnemy::PerformHitTrace, AttackDuration*0.5f, false);

    FTimerHandle FinishTimer;
    GetWorldTimerManager().SetTimer(FinishTimer, this, &ACPP_BaseEnemy::FinishAttack, AttackDuration, false);
}

void ACPP_BaseEnemy::PerformHitTrace()
{
    if (!CharacterStats) return;

    float ScaledDamage = CharacterStats->LightAttackDamage * EnemyLevelDamageMultiplier;

    PerformAttackTrace(
        CharacterStats->LightAttackRange,
        CharacterStats->LightAttackBoxSize,
        ScaledDamage);
}

void ACPP_BaseEnemy::FinishAttack()
{
    bIsAttacking = false;
    GetSprite()->SetLooping(true);
    GetSprite()->Play();
}

bool ACPP_BaseEnemy::CanDealDamageTo(AActor* TargetActor) const
{
    if (!Super::CanDealDamageTo(TargetActor))
    {
        return false;
    }

    if (TargetActor->IsA(ACPP_BaseEnemy::StaticClass()))
    {
        return false;
    }

    return true;
}

void ACPP_BaseEnemy::OnDeath_Implementation()
{
    SpawnLoot();
    SpawnCoins();
    SpawnXP();

    Super::OnDeath_Implementation();
}

void ACPP_BaseEnemy::SpawnLoot()
{
    if (!CharacterStats) return;

    if (FMath::FRand() > CharacterStats->DropChance) return;

    if (CharacterStats->LootTable.Num() == 0) return;

    int32 ItemsToSpawn = FMath::RandRange(CharacterStats->MinLootCount, CharacterStats->MaxLootCount);

    //assuming singleplayer
    ACPP_BaseCharacter* Player = Cast<ACPP_BaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;
    //

    for (int32 i = 0; i < ItemsToSpawn; i++)
    {
        TArray<FLootItem> ValidCandidates;
        float ValidTotalWeight = 0.0f;

        for (const FLootItem& Entry : CharacterStats->LootTable)
        {
            if (!Entry.ItemClass) continue;

            ACPP_BaseItem* DefaultItem = Cast<ACPP_BaseItem>(Entry.ItemClass->GetDefaultObject());

            bool bIsItemValid = true;

            if (DefaultItem)
            {
                FGameplayTag Tag = DefaultItem->GetAbilityTag();

                if (Tag.IsValid())
                {
                    int32 CurrentLevel = Player->GetAbilityLevel(Tag);
                    int32 MaxLevel = DefaultItem->GetMaxLevel();

                    if (CurrentLevel >= MaxLevel)
                    {
                        bIsItemValid = false;
                    }
                }
            }

            if (bIsItemValid)
            {
                ValidCandidates.Add(Entry);
                ValidTotalWeight += Entry.DropWeight;
            }
        }

        if (ValidCandidates.Num() == 0) return;
        if (ValidTotalWeight <= 0.0f) return;

        float RandomPoint = FMath::FRandRange(0.0f, ValidTotalWeight);
        float CurrentSum = 0.0f;
        TSubclassOf<ACPP_BaseItem> SelectedItemClass = nullptr;

        for (const FLootItem& ValidEntry : ValidCandidates)
        {
            CurrentSum += ValidEntry.DropWeight;
            if (RandomPoint <= CurrentSum)
            {
                SelectedItemClass = ValidEntry.ItemClass;
                break;
            }
        }

        if (SelectedItemClass)
        {
            FVector SpawnLocation = GetActorLocation();
            SpawnLocation.Z += 20.0f;
            SpawnLocation.X += FMath::RandRange(-10.f, 10.f);
            SpawnLocation.Y += FMath::RandRange(-10.f, 10.f);

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            ACPP_BaseItem* DroppedItem = GetWorld()->SpawnActor<ACPP_BaseItem>(
                SelectedItemClass,
                SpawnLocation,
                FRotator::ZeroRotator,
                SpawnParams
                );

            if (DroppedItem)
            {
                UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent());
                if (RootPrim)
                {
                    FVector ImpulseDir = FVector(
                        FMath::RandRange(-1.f, 1.f),
                        FMath::RandRange(-1.f, 1.f),
                        FMath::RandRange(0.5f, 1.5f)
                    );
                    ImpulseDir.Normalize();

                    float ImpulseStrength = FMath::RandRange(300.0f, 500.0f);
                    RootPrim->AddImpulse(ImpulseDir * ImpulseStrength, NAME_None, true);
                }
            }
        }
    }
}

void ACPP_BaseEnemy::SpawnCoins()
{
    if (!CharacterStats || !CharacterStats->CoinClass) return;

    int32 TotalValue = FMath::RandRange(CharacterStats->MinCoins, CharacterStats->MaxCoins);
    if (TotalValue <= 0) return;

    FVector SpawnLocation = GetActorLocation();
    SpawnLocation.Z += 40.0f;

    SpawnLocation.X += FMath::RandRange(-20.f, 20.f);
    SpawnLocation.Y += FMath::RandRange(-20.f, 20.f);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ACPP_Item_Currency* DroppedCoin = GetWorld()->SpawnActor<ACPP_Item_Currency>(
        CharacterStats->CoinClass,
        SpawnLocation,
        FRotator::ZeroRotator,
        Params
        );

    if (DroppedCoin)
    {
        DroppedCoin->SetValue(TotalValue);

        UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(DroppedCoin->GetRootComponent());
        if (RootPrim)
        {
            FVector Impulse = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 1.0f);
            RootPrim->AddImpulse(Impulse.GetSafeNormal() * 400.0f, NAME_None, true);
        }
    }
}

void ACPP_BaseEnemy::SpawnXP()
{
    if (!CharacterStats || !CharacterStats->XPItemClass) return;

    float BaseAmount = (float)FMath::RandRange(CharacterStats->MinXP, CharacterStats->MaxXP);

    float XPScaler = 1.0f;
    ACPP_BaseCharacter* Player = Cast<ACPP_BaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Player)
    {
        int32 PlayerLevel = Player->GetCharacterLevel();
        if (PlayerLevel > 1)
        {
            XPScaler = 1.0f + ((PlayerLevel - 1) * CharacterStats->XPScalingFactor);
        }
    }

    int32 FinalXPAmount = FMath::RoundToInt(BaseAmount * XPScaler);

    if (FinalXPAmount <= 0) return;

    FVector SpawnLocation = GetActorLocation();
    SpawnLocation.Z += 50.0f;
    SpawnLocation.X += FMath::RandRange(-30.f, 30.f);
    SpawnLocation.Y += FMath::RandRange(-30.f, 30.f);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ACPP_Item_XP* DroppedXP = GetWorld()->SpawnActor<ACPP_Item_XP>(
        CharacterStats->XPItemClass,
        SpawnLocation,
        FRotator::ZeroRotator,
        Params
        );

    if (DroppedXP)
    {
        // 4. Устанавливаем значение
        DroppedXP->SetValue(FinalXPAmount);

        // 5. Импульс (чуть сильнее вверх, чем монеты)
        UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(DroppedXP->GetRootComponent());
        if (RootPrim)
        {
            FVector Impulse = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 1.5f); // Z выше
            RootPrim->AddImpulse(Impulse.GetSafeNormal() * 450.0f, NAME_None, true);
        }
    }
}
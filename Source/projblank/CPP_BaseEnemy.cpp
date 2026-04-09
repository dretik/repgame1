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
#include "CPP_GameInstance.h"
#include "CPP_SaveGame.h"
#include "CPP_Item_SkillUnlockable.h"
#include "CPP_Action.h"   
#include "CPP_ActionComponent.h"
#include "CPP_PlayerCharacter.h"

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

    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

    if (PawnSensingComp)
    {
        PawnSensingComp->OnSeePawn.AddDynamic(this, &ACPP_BaseEnemy::OnPawnSeen);
    }

    if (Player && CharacterStats && AttributeComp)
    {
        int32 PlayerLevel = Player->GetCharacterLevel();

        if (PlayerLevel > 1)
        {
            float HealthScale = 1.0f + ((PlayerLevel - 1) * CharacterStats->HealthScalingFactor);
            float NewMaxHealth = CharacterStats->MaxHealth * HealthScale;

            AttributeComp->InitializeStats(NewMaxHealth);

            EnemyLevelDamageMultiplier = 1.0f + ((PlayerLevel - 1) * CharacterStats->DamageScalingFactor);
        }
    }

    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
    if (GI && GI->bIsLoadingSave)
    {
        UCPP_SaveGame* LoadInst = Cast<UCPP_SaveGame>(UGameplayStatics::LoadGameFromSlot(GI->SaveSlotName, 0));

        if (LoadInst)
        {
            FString MyID = GetName();

            if (LoadInst->WorldEnemies.Contains(MyID))
            {
                FEnemySaveData Data = LoadInst->WorldEnemies[MyID];

                if (Data.bIsDead)
                {
                    Destroy();
                }
                else
                {
                    SetActorLocation(Data.Location);

                    if (AttributeComp)
                    {
                        float CurrentHP = AttributeComp->GetHealth();
                        float Diff = Data.CurrentHealth - CurrentHP;

                        AttributeComp->ApplyHealthChange(nullptr, Diff);
                    }
                }
            }
        }
    }
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
}

void ACPP_BaseEnemy::AttackPlayer()
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("AI Trying to Attack..."));
    if (!CharacterStats) return;
    if (IsDead() || GetIsAttacking()) return;
    if (!ActionComp) return;
    static FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag("Ability.Enemy.Melee");

    ActionComp->StartActionByName(this, AttackTag);
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
    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;

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
                ACPP_Item_SkillUnlockable* SkillItem = Cast<ACPP_Item_SkillUnlockable>(DefaultItem);

                if (SkillItem)
                {
                    if (SkillItem->GetActionClass())
                    {
                        UCPP_Action* DefaultAction = SkillItem->GetActionClass()->GetDefaultObject<UCPP_Action>();

                        if (DefaultAction)
                        {
                            FGameplayTag Tag = DefaultAction->ActionTag;

                            if (Tag.IsValid())
                            {
                                int32 CurrentLevel = Player->GetAbilityLevel(Tag);

                                if (CurrentLevel >= SkillItem->GlobalMaxSkillLevel)
                                {
                                    bIsItemValid = false;
                                }
                            }
                        }
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

    float CoinScaler = 1.0f;
    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Player)
    {
        int32 PlayerLevel = Player->GetCharacterLevel();
        if (PlayerLevel > 1)
        {
            CoinScaler = 1.0f + ((PlayerLevel - 1) * CharacterStats->CoinScalingFactor);
        }
    }

    float ScaledMinCoins = (float)CharacterStats->MinCoins * CoinScaler;
    float ScaledMaxCoins = (float)CharacterStats->MaxCoins * CoinScaler;

    int32 TotalValue = FMath::RandRange(ScaledMinCoins, ScaledMaxCoins);
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
    float XPScaler = 1.0f;
    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Player)
    {
        int32 PlayerLevel = Player->GetCharacterLevel();
        if (PlayerLevel > 1)
        {
            XPScaler = 1.0f + ((PlayerLevel - 1) * CharacterStats->XPScalingFactor);
        }
    }

    float ScaledMinXP = (float)CharacterStats->MinXP * XPScaler;
    float ScaledMaxXP = (float)CharacterStats->MaxXP * XPScaler;

    float BaseAmount = (float)FMath::RandRange(ScaledMinXP, ScaledMaxXP);

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
        DroppedXP->SetValue(FinalXPAmount);

        UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(DroppedXP->GetRootComponent());
        if (RootPrim)
        {
            FVector Impulse = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 1.5f);
            RootPrim->AddImpulse(Impulse.GetSafeNormal() * 450.0f, NAME_None, true);
        }
    }
}

void ACPP_BaseEnemy::OnSaveGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject) return;

    if (IsDead())
    {
        if (!bIsDynamicallySpawned)
        {
            FEnemySaveData Data;
            Data.bIsDead = true;
            SaveObject->WorldEnemies.Add(GetName(), Data);
        }
    }
    else
    {
        if (bIsDynamicallySpawned)
        {
            FDynamicEnemyData DynData;
            DynData.EnemyClass = GetClass();
            DynData.Transform = GetActorTransform();
            DynData.CurrentHealth = AttributeComp ? AttributeComp->GetHealth() : 0.0f;
            SaveObject->SpawnedEnemies.Add(DynData);
        }
        else
        {
            FEnemySaveData Data;
            Data.CurrentHealth = AttributeComp ? AttributeComp->GetHealth() : 0.0f;
            Data.Location = GetActorLocation();
            Data.bIsDead = false;
            SaveObject->WorldEnemies.Add(GetName(), Data);
        }
    }
}

void ACPP_BaseEnemy::OnLoadGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject) return;

    FString MyID = GetName();

    if (SaveObject->WorldEnemies.Contains(MyID))
    {
        FEnemySaveData Data = SaveObject->WorldEnemies[MyID];

        if (Data.bIsDead)
        {
            Destroy();
        }
        else
        {
            SetActorLocation(Data.Location);
            if (AttributeComp)
            {
                float CurrentHP = AttributeComp->GetHealth();
                float Diff = Data.CurrentHealth - CurrentHP;
                AttributeComp->ApplyHealthChange(nullptr, Diff);
            }
        }
    }
}
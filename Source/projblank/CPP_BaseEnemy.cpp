#include "CPP_BaseEnemy.h"
#include "Perception/PawnSensingComponent.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
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
#include "CPP_VisualComponent.h"
#include "CPP_ProgressionStatics.h"
#include "CPP_LootStatics.h"

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
        GetCharacterMovement()->bImpartBaseAngularVelocity = false;
        bUseControllerRotationYaw = false;
        bUseControllerRotationPitch = false;
        bUseControllerRotationRoll = false;
    }

}

void ACPP_BaseEnemy::BeginPlay()
{
    Super::BeginPlay();

    if (PawnSensingComp)
    {
        PawnSensingComp->OnSeePawn.AddDynamic(this, &ACPP_BaseEnemy::OnPawnSeen);
    }

    float InitialYaw = bStartFacingRight ? 90.0f : -90.0f;
    SetActorRotation(FRotator(0, InitialYaw, 0));

    if (VisualComp) {
        VisualComp->FaceLocation(GetActorLocation() + GetActorForwardVector() * 100.0f, BaseSpriteScale);
    }

    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
    if (GI)
    {
        GI->OnWorldLevelUp.AddDynamic(this, &ACPP_BaseEnemy::OnWorldLevelChanged);
    }

    //if (!bStatsRestoredFromSave)
    //{
    //    InitializeEnemyScaling();
    //}

    InitializeEnemyScaling();
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

bool ACPP_BaseEnemy::CanDealDamageTo_Implementation(AActor* TargetActor) const
{
    if (!Super::CanDealDamageTo_Implementation(TargetActor))
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
    UCPP_LootStatics::SpawnAllLoot(this, CharacterStats, GetActorLocation());

    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
    if (GI && CharacterStats)
    {
        GI->AddWorldExperience(CharacterStats->MinXP);
    }

    Super::OnDeath_Implementation();
}

void ACPP_BaseEnemy::InitializeEnemyScaling()
{
    if (CharacterStats && AttributeComp)
    {
        int32 DiffLevel = UCPP_ProgressionStatics::GetCurrentDifficultyLevel(this);

        float NewMaxHealth = UCPP_ProgressionStatics::CalculateEnemyHealth(CharacterStats->MaxHealth, CharacterStats->HealthScalingFactor, DiffLevel);
        AttributeComp->InitializeStats(NewMaxHealth);

        float NewMult = UCPP_ProgressionStatics::CalculateEnemyDamageMultiplier(CharacterStats->DamageScalingFactor, DiffLevel);
        AttributeComp->SetDamageMultiplier(NewMult);
    }
}

void ACPP_BaseEnemy::OnSaveGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject) return;

        if (Tags.Contains(FName("Dynamic")))
        {
            if (!IsDead()) {
                FDynamicEnemyData DynData;
                DynData.EnemyClass = GetClass();
                DynData.Transform = GetActorTransform();
                DynData.CurrentHealth = AttributeComp ? AttributeComp->GetHealth() : 0.0f;
                DynData.MaxHealth = AttributeComp ? AttributeComp->GetMaxHealth() : 0.0f;
                DynData.SavedDamageMultiplier = AttributeComp ? AttributeComp->GetDamageMultiplier() : 1.0f;
                SaveObject->SpawnedEnemies.Add(DynData);
            }
        }
        else
        {
            if (!IsDead()) {
                FEnemySaveData Data;
                Data.CurrentHealth = AttributeComp ? AttributeComp->GetHealth() : 0.0f;
                Data.MaxHealth = AttributeComp ? AttributeComp->GetMaxHealth() : 0.0f;
                Data.SavedDamageMultiplier = AttributeComp ? AttributeComp->GetDamageMultiplier() : 1.0f;
                Data.Location = GetActorLocation();
                Data.bIsDead = false;
                SaveObject->WorldEnemies.Add(GetName(), Data);
            }
        }
}

void ACPP_BaseEnemy::OnLoadGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject) return;
    bStatsRestoredFromSave = true;
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
                float MaxHP = (Data.MaxHealth > 0.0f) ? Data.MaxHealth : (CharacterStats ? CharacterStats->MaxHealth : 100.0f);
                AttributeComp->InitializeStats(MaxHP);

                float Mult = (Data.SavedDamageMultiplier > 0.0f) ? Data.SavedDamageMultiplier : 1.0f;
                AttributeComp->SetDamageMultiplier(Mult);

                float Diff = Data.CurrentHealth - AttributeComp->GetHealth();
                AttributeComp->ApplyHealthChange(nullptr, Diff);
            }
        }
    }
}

void ACPP_BaseEnemy::OnWorldLevelChanged(int32 NewLevel)
{
    if (bIsDead) return;

    // current hp
    float HealthPercent = AttributeComp->GetHealth() / AttributeComp->GetMaxHealth();

    // recalc
    InitializeEnemyScaling();

    // restore
    float NewHealth = AttributeComp->GetMaxHealth() * HealthPercent;
    float Diff = NewHealth - AttributeComp->GetHealth();
    AttributeComp->ApplyHealthChange(nullptr, Diff);

    if (VisualComp) VisualComp->PlayHitFlash(0.5f, FLinearColor::Red);
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_BaseEnemy.h"
#include "Perception/PawnSensingComponent.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "CPP_BaseItem.h"
#include "CharacterStats.h"

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

    if (PawnSensingComp)
    {
        PawnSensingComp->OnSeePawn.AddDynamic(this, &ACPP_BaseEnemy::OnPawnSeen);
    }
}

void ACPP_BaseEnemy::OnPawnSeen(APawn* SeenPawn)
{
    if (SeenPawn == nullptr) return;

    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SeenPawn);
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
    if (!CharacterStats) return;
    if (bIsAttacking) return;

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

    PerformAttackTrace(
        CharacterStats->LightAttackRange,
        CharacterStats->LightAttackBoxSize,
        CharacterStats->LightAttackDamage);
}

void ACPP_BaseEnemy::FinishAttack()
{
    bIsAttacking = false;
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

    Super::OnDeath_Implementation();
}

void ACPP_BaseEnemy::SpawnLoot()
{
    if (!CharacterStats) return;

    if (FMath::FRand() > CharacterStats->DropChance)
    {
        return;
    }

    if (CharacterStats->PossibleDrops.Num() == 0)
    {
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, CharacterStats->PossibleDrops.Num() - 1);
    TSubclassOf<ACPP_BaseItem> ItemToSpawn = CharacterStats->PossibleDrops[RandomIndex];

    if (ItemToSpawn)
    {
        FVector SpawnLocation = GetActorLocation();
        SpawnLocation.Z += 10.0f;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACPP_BaseItem* DroppedItem = GetWorld()->SpawnActor<ACPP_BaseItem>(
            ItemToSpawn,
            SpawnLocation,
            FRotator::ZeroRotator,
            SpawnParams
            );

        // impulse
        if (DroppedItem)
        {
            UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent());
            if (RootPrim)
            {
                FVector ImpulseDirection = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 1.0f);
                ImpulseDirection.Normalize();
                RootPrim->AddImpulse(ImpulseDirection * 300.0f, NAME_None, true);
            }
        }
    }
}
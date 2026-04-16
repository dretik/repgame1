#include "CPP_LootStatics.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_BaseItem.h"
#include "CPP_Item_SkillUnlockable.h"
#include "CPP_Item_Currency.h"
#include "CPP_Item_XP.h"
#include "CPP_ActionComponent.h"
#include "CPP_ProgressionStatics.h"

void UCPP_LootStatics::SpawnAllLoot(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location)
{
    if (!Stats) return;

    if (FMath::FRand() > Stats->DropChance) return;

    ExecuteSpawnLootTable(WorldContextObject, Stats, Location);
    ExecuteSpawnCurrency(WorldContextObject, Stats, Location);
    ExecuteSpawnXP(WorldContextObject, Stats, Location);
}

void UCPP_LootStatics::ExecuteSpawnLootTable(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location)
{
    if (FMath::FRand() > Stats->DropChance || Stats->LootTable.Num() == 0) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(WorldContextObject, 0);
    UCPP_ActionComponent* PlayerActionComp = PlayerChar ? PlayerChar->FindComponentByClass<UCPP_ActionComponent>() : nullptr;

    int32 ItemsToSpawn = FMath::RandRange(Stats->MinLootCount, Stats->MaxLootCount);

    for (int32 i = 0; i < ItemsToSpawn; i++)
    {
        TArray<FLootItem> ValidCandidates;
        float TotalWeight = 0.0f;

        FVector FinalLocation = Location;
        FinalLocation.Z += 20.0f;
        FinalLocation.X += FMath::RandRange(-15.f, 15.f);
        FinalLocation.Y += FMath::RandRange(-15.f, 15.f);

        for (const FLootItem& Entry : Stats->LootTable)
        {
            if (!Entry.ItemClass) continue;

            bool bIsValid = true;
            //skill item filter
            if (Entry.ItemClass->IsChildOf(ACPP_Item_SkillUnlockable::StaticClass()) && PlayerActionComp)
            {
                ACPP_Item_SkillUnlockable* SkillItem = Entry.ItemClass->GetDefaultObject<ACPP_Item_SkillUnlockable>();
                UCPP_Action* DefaultAction = SkillItem->GetActionClass() ? SkillItem->GetActionClass()->GetDefaultObject<UCPP_Action>() : nullptr;

                if (DefaultAction && PlayerActionComp->GetActionLevel(DefaultAction->ActionTag) >= DefaultAction->MaxLevel)
                {
                    bIsValid = false;
                }
            }

            if (bIsValid)
            {
                ValidCandidates.Add(Entry);
                TotalWeight += Entry.DropWeight;
            }
        }

        if (ValidCandidates.Num() == 0 || TotalWeight <= 0.0f) continue;

        float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
        float CurrentSum = 0.0f;
        for (const FLootItem& ValidEntry : ValidCandidates)
        {
            CurrentSum += ValidEntry.DropWeight;
            if (RandomPoint <= CurrentSum)
            {
                AActor* DroppedItem = WorldContextObject->GetWorld()->SpawnActor<AActor>(ValidEntry.ItemClass, Location + FVector(0, 0, 20), FRotator::ZeroRotator);
                ApplyLootImpulse(DroppedItem);
                break;
            }
        }
    }
}

void UCPP_LootStatics::ExecuteSpawnCurrency(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location)
{
    if (!Stats || !Stats->CoinClass) return;

    int32 Diff = UCPP_ProgressionStatics::GetCurrentDifficultyLevel(WorldContextObject);
    int32 Amount = UCPP_ProgressionStatics::CalculateDroppedCoins(Stats->MinCoins, Stats->MaxCoins, Stats->CoinScalingFactor, Diff);

    if (Amount > 0)
    {
        FVector FinalLocation = Location;
        FinalLocation.Z += 40.0f; 
        FinalLocation.X += FMath::RandRange(-25.f, 25.f);
        FinalLocation.Y += FMath::RandRange(-25.f, 25.f);

        ACPP_Item_Currency* Coin = WorldContextObject->GetWorld()->SpawnActor<ACPP_Item_Currency>(Stats->CoinClass, Location + FVector(0, 0, 40), FRotator::ZeroRotator);
        if (Coin) {
            Coin->SetValue(Amount);
            ApplyLootImpulse(Coin);
        }
    }
}

void UCPP_LootStatics::ExecuteSpawnXP(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location)
{
    if (!Stats || !Stats->XPItemClass) return;

    int32 Diff = UCPP_ProgressionStatics::GetCurrentDifficultyLevel(WorldContextObject);
    int32 Amount = UCPP_ProgressionStatics::CalculateDroppedXP(Stats->MinXP, Stats->MaxXP, Stats->XPScalingFactor, Diff);

    if (Amount > 0)
    {
        FVector FinalLocation = Location;
        FinalLocation.Z += 50.0f;
        FinalLocation.X += FMath::RandRange(-25.f, 25.f);
        FinalLocation.Y += FMath::RandRange(-25.f, 25.f);

        ACPP_Item_XP* XPItem = WorldContextObject->GetWorld()->SpawnActor<ACPP_Item_XP>(Stats->XPItemClass, Location + FVector(0, 0, 50), FRotator::ZeroRotator);
        if (XPItem) {
            XPItem->SetValue(Amount);
            ApplyLootImpulse(XPItem);
        }
    }
}

void UCPP_LootStatics::ApplyLootImpulse(AActor* Item)
{
    if (!Item) return;
    Item->Tags.Add(FName("Dynamic"));
    UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Item->GetRootComponent());
    if (Root)
    {
        FVector Impulse = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), FMath::RandRange(1.f, 1.5f));
        Root->AddImpulse(Impulse.GetSafeNormal() * FMath::RandRange(300.f, 500.f), NAME_None, true);
    }
}
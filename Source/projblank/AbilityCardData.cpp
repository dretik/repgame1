// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityCardData.h"
#include "CPP_Action.h"

TArray<FAbilityCard> UAbilityCardPool::GetRandomCards(int32 Count, const TArray<FGameplayTag>& ExcludeTags)
{
    TArray<FAbilityCard> Result;

    // 1. copy in temp for filter
    TArray<FAbilityCard> ValidCards = AllCards;

    //filter existing or maxed out
    for (int32 i = ValidCards.Num() - 1; i >= 0; --i)
    {
        if (ValidCards[i].ActionClass)
        {
            UCPP_Action* DefaultObj = ValidCards[i].ActionClass->GetDefaultObject<UCPP_Action>();
            if (DefaultObj && ExcludeTags.Contains(DefaultObj->ActionTag))
            {
                ValidCards.RemoveAt(i);
            }
        }
    }

    if (ValidCards.Num() == 0) return Result;

    //rarity 
    //float CardWeight = Card.DropWeight;
    //if (Card.Rarity == ECardRarity::Legendary) CardWeight *= PlayerLuck;

    //weighted random
    for (int32 Iter = 0; Iter < Count; ++Iter)
    {
        if (ValidCards.Num() == 0) break;

        float TotalWeight = 0.0f;
        for (const FAbilityCard& Card : ValidCards)
        {
            TotalWeight += Card.DropWeight;
        }

        float RandomWeight = FMath::FRandRange(0.0f, TotalWeight);
        float Cursor = 0.0f;

        for (int32 i = 0; i < ValidCards.Num(); ++i)
        {
            Cursor += ValidCards[i].DropWeight;
            if (Cursor >= RandomWeight)
            {
                //add picked card to result
                Result.Add(ValidCards[i]);

                // remove form temp (to prevent picking twice)
                ValidCards.RemoveAt(i);
                break;
            }
        }
    }

    return Result;
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityCardData.generated.h"

class UCPP_Action;

UENUM(BlueprintType)
enum class ECardRarity : uint8
{
    Common,
    Rare,
    Epic,
    Legendary
};

UENUM(BlueprintType)
enum class ECardType : uint8
{
    ActiveAbility, // ability (Fireball, Dash)
    StatUpgrade,   // params (HP+, Damage+)
    PassiveEffect  // passive (Шанс крита, Вампиризм)
};

USTRUCT(BlueprintType)
struct FAbilityCard
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        ECardType CardType = ECardType::ActiveAbility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        ECardRarity Rarity = ECardRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
        FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        UTexture2D* Icon;

    //actionclass if ability
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "CardType != ECardType::StatUpgrade"))
        TSubclassOf<UCPP_Action> ActionClass;

    //stattag and value if passive upgrade
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "CardType == ECardType::StatUpgrade"))
        FGameplayTag StatTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "CardType == ECardType::StatUpgrade"))
        float StatModifierValue = 0.0f;

    // card pull appearence weight
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        float DropWeight = 1.0f;
};

UCLASS()
class PROJBLANK_API UAbilityCardPool : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool")
        TArray<FAbilityCard> AllCards;

    // random cards from card pull
    UFUNCTION(BlueprintCallable, Category = "Roguelike")
        TArray<FAbilityCard> GetRandomCards(int32 Count, const TArray<FGameplayTag>& ExcludeTags);
};
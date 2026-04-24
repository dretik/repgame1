// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CPP_AttributeComponent.h"
#include "RarityTypes.h"
#include "AbilityCardData.generated.h"

class UCPP_Action;

UENUM(BlueprintType)
enum class ECardType : uint8
{
    ActiveAbility, // ability (Fireball, Dash)
    StatUpgrade,   // params (HP+, Damage+)
    PassiveEffect  // passive (crit chance, lifesteal)
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
        UMaterialInterface* IconMaterial;

    //actionclass if ability
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        TSubclassOf<UCPP_Action> ActionClass;

    //statmodifierds array
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        TArray<FStatModifier> StatModifiers;

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
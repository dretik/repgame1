// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "RarityTypes.generated.h"

UENUM(BlueprintType)
enum class ECardRarity : uint8
{
    Common    UMETA(DisplayName = "Common"),
    Rare      UMETA(DisplayName = "Rare"),
    Epic      UMETA(DisplayName = "Epic"),
    Legendary UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		bool bIsMultiplier = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float Value = 0.0f;
};
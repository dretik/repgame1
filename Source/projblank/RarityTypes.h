// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RarityTypes.generated.h"

UENUM(BlueprintType)
enum class ECardRarity : uint8
{
    Common    UMETA(DisplayName = "Common"),
    Rare      UMETA(DisplayName = "Rare"),
    Epic      UMETA(DisplayName = "Epic"),
    Legendary UMETA(DisplayName = "Legendary")
};
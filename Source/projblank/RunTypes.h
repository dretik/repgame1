// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTypes.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest,
    Dungeon,
    Castle
};

USTRUCT(BlueprintType)
struct FLevelData
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EBiomeType Biome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bIsBossLevel = false;

    //difficulty or rules for levels could be added
};
// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_ProgressionStatics.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_PlayerCharacter.h"

int32 UCPP_ProgressionStatics::GetCurrentDifficultyLevel(const UObject* WorldContextObject)
{
    // player level for now
    // could take "ZoneLevel" from GameInstance
    if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(WorldContextObject, 0))
    {
        if (ACPP_PlayerCharacter* MyPlayer = Cast<ACPP_PlayerCharacter>(PlayerChar))
        {
            return MyPlayer->GetCharacterLevel();
        }
    }
    return 1; //if player is miising (Fallback)
}

float UCPP_ProgressionStatics::CalculateEnemyHealth(float BaseHealth, float ScalingFactor, int32 DifficultyLevel)
{
    if (DifficultyLevel <= 1) return BaseHealth;
    float HealthScale = 1.0f + ((DifficultyLevel - 1) * ScalingFactor);
    return BaseHealth * HealthScale;
}

float UCPP_ProgressionStatics::CalculateEnemyDamageMultiplier(float ScalingFactor, int32 DifficultyLevel)
{
    if (DifficultyLevel <= 1) return 1.0f;
    return 1.0f + ((DifficultyLevel - 1) * ScalingFactor);
}

int32 UCPP_ProgressionStatics::CalculateDroppedXP(int32 MinXP, int32 MaxXP, float ScalingFactor, int32 DifficultyLevel)
{
    float XPScaler = (DifficultyLevel > 1) ? 1.0f + ((DifficultyLevel - 1) * ScalingFactor) : 1.0f;
    float ScaledMinXP = (float)MinXP * XPScaler;
    float ScaledMaxXP = (float)MaxXP * XPScaler;

    float BaseAmount = (float)FMath::RandRange(ScaledMinXP, ScaledMaxXP);
    return FMath::RoundToInt(BaseAmount * XPScaler);
}

int32 UCPP_ProgressionStatics::CalculateDroppedCoins(int32 MinCoins, int32 MaxCoins, float ScalingFactor, int32 DifficultyLevel)
{
    float CoinScaler = (DifficultyLevel > 1) ? 1.0f + ((DifficultyLevel - 1) * ScalingFactor) : 1.0f;
    float ScaledMinCoins = (float)MinCoins * CoinScaler;
    float ScaledMaxCoins = (float)MaxCoins * CoinScaler;

    return FMath::RandRange(ScaledMinCoins, ScaledMaxCoins);
}

float UCPP_ProgressionStatics::CalculateRequiredXPForNextLevel(float CurrentRequiredXP, float LevelUpMultiplier)
{
    // lvl1: 100 * (1.2^0) = 100
    // lvl2: 100 * (1.2^1) = 120
    // lvl3: 100 * (1.2^2) = 144
    //return 100.0f * FMath::Pow(LevelUpMultiplier, TargetLevel - 1);
    return CurrentRequiredXP * LevelUpMultiplier;
}
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CPP_ProgressionStatics.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API UCPP_ProgressionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    // global difficulty
    // difficulty level from player for now, could be moved to  GameInstance)
    UFUNCTION(BlueprintPure, Category = "Progression", meta = (WorldContext = "WorldContextObject"))
        static int32 GetCurrentDifficultyLevel(const UObject* WorldContextObject);

    //enemy scalers
    UFUNCTION(BlueprintPure, Category = "Progression")
        static float CalculateEnemyHealth(float BaseHealth, float ScalingFactor, int32 DifficultyLevel);

    UFUNCTION(BlueprintPure, Category = "Progression")
        static float CalculateEnemyDamageMultiplier(float ScalingFactor, int32 DifficultyLevel);

    //loot
    UFUNCTION(BlueprintPure, Category = "Progression")
        static int32 CalculateDroppedXP(int32 MinXP, int32 MaxXP, float ScalingFactor, int32 DifficultyLevel);

    UFUNCTION(BlueprintPure, Category = "Progression")
        static int32 CalculateDroppedCoins(int32 MinCoins, int32 MaxCoins, float ScalingFactor, int32 DifficultyLevel);

    //player scaling
    UFUNCTION(BlueprintPure, Category = "Progression")
        static float CalculateRequiredXPForNextLevel(float CurrentRequiredXP, float LevelUpMultiplier);
};

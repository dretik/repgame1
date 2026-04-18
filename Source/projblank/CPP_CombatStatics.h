// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CPP_CombatStatics.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API UCPP_CombatStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:

public:
    UFUNCTION(BlueprintCallable, Category = "Combat|Statics")
        static bool ExecuteBoxTraceAttack(
            AActor* DamageCauser,
            AActor* Instigator,
            FVector Origin,
            FVector AttackDirection,
            float Range,
            FVector BoxSize,
            float BaseDamage,
            bool bDrawDebug = false);

    UFUNCTION(BlueprintCallable, Category = "Combat|Statics")
        static bool ExecuteAreaDamage(
            AActor* DamageCauser,
            AActor* Instigator,
            FVector Origin,
            float Radius,
            float BaseDamage,
            float ImpulseStrength=500.f,
            bool bDrawDebug = false);

    static bool ExecuteHealing(AActor* Instigator, AActor* Target, float HealAmount, bool bIsPercentage);
};

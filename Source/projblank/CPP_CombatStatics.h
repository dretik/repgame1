// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CPP_CombatStatics.generated.h"

class UCPP_Action_Effect;

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
            const TArray<TSubclassOf<UCPP_Action_Effect>>& EffectsToApply,
            float ImpulseStrength = 500.0f,
            bool bDrawDebug = false
        );

    UFUNCTION(BlueprintCallable, Category = "Combat|Statics")
        static bool ExecuteAreaDamage(
            AActor* DamageCauser,
            AActor* Instigator,
            FVector Origin,
            float Radius,
            float BaseDamage,
            const TArray<TSubclassOf<UCPP_Action_Effect>>& EffectsToApply,
            float ImpulseStrength= 500.0f,
            bool bDrawDebug = false
        );

    UFUNCTION(BlueprintCallable, Category = "Combat|Statics")
        static ACPP_Projectile* SpawnProjectile(
            AActor* Instigator,
            TSubclassOf<ACPP_Projectile> ProjectileClass,
            float Damage,
            float Speed,
            float Radius,
            float Scale,
            const TArray<TSubclassOf<UCPP_Action_Effect>>& EffectsToApply
        );

    static bool ExecuteHealing(AActor* Instigator, AActor* Target, float HealAmount, bool bIsPercentage);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterStats.generated.h"

UCLASS()
class PROJBLANK_API UCharacterStats : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
    // health
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Health")
        float MaxHealth = 100.0f;

    // attacks
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat")
        float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat")
        float HeavyAttackMultiplier = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat")
        float AttackRange = 150.0f;

    // movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float MaxWalkSpeed = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float JumpVelocity = 650.0f;

    //overall
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Overall")
        float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Overall")
        float AirControl = 1.0f;
};

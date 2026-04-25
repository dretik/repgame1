// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CPP_InventoryComponent.h"
#include "GameplayTagContainer.h"
#include "CPP_SaveGame.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FEnemySaveData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
        float CurrentHealth = 0.0f;

    UPROPERTY(VisibleAnywhere) 
        float MaxHealth = 0.0f;

    UPROPERTY(VisibleAnywhere)
        FVector Location = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere)
        bool bIsDead = false;

    UPROPERTY(VisibleAnywhere)
        float SavedDamageMultiplier=1.f;
};

USTRUCT(BlueprintType)
struct FDynamicEnemyData
{
    GENERATED_BODY()

        UPROPERTY(VisibleAnywhere)
        TSubclassOf<class AActor> EnemyClass;

    UPROPERTY(VisibleAnywhere)
        FTransform Transform; 

    UPROPERTY(VisibleAnywhere)
        float CurrentHealth = 0.0f;

    UPROPERTY(VisibleAnywhere)
        float MaxHealth = 0.0f;

    UPROPERTY(VisibleAnywhere)
        float SavedDamageMultiplier=1.f;
};

UCLASS()
class PROJBLANK_API UCPP_SaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
    UCPP_SaveGame();

    // --- PLAYER STATS ---
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float Health;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float MaxHealth;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float BaseDamage;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        int32 Level;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float CurrentXP;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float XPToNextLevel;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        int32 Coins;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float DamageMultiplier;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        float BaseSpeed;

    // --- POSITION ---
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        FVector PlayerLocation;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        FName LevelName;

        // --- INVENTORY & ABILITIES ---
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        TArray<FInventorySlot> InventoryData;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        TMap<FGameplayTag, int32> AbilityLevels;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        TMap<FGameplayTag, TSubclassOf<class UCPP_Action>> EquippedSlots;

    // --- WORLD DATA ---

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        TMap<FString, FEnemySaveData> WorldEnemies;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        TArray<FDynamicEnemyData> SpawnedEnemies;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
        TSet<FString> CollectedItems;
};

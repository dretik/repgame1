// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterStats.generated.h"

class ACPP_BaseItem;
class UNiagaraSystem;
class ACPP_Item_Currency;

USTRUCT(BlueprintType)
struct FLootItem
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<ACPP_BaseItem> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
        float DropWeight = 1.0f;
};

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

    //hitbox
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat")
        float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat")
        FVector AttackBoxSize = FVector(50.0f, 50.0f, 50.0f);

    //combat anims durations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Durations")
        float Attack1Duration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Durations")
        float Attack2Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Durations")
        float Attack3Duration = 0.6f;

    //movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float MaxWalkSpeed = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float JumpVelocity = 650.0f;

    //abilties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float DodgeStrength = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float DodgeDuration = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float JumpDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float DashAttackImpulse = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Movement")
        float HeavyAttackWalkSpeed = 50.0f;

    //physics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Overall")
        float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Overall")
        float AirControl = 1.0f;

    // combat configs
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat")
        float ComboResetTime = 0.3f;

    // --- LIGHT ATTACK (Phase 1) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Light Attack")
        float LightAttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Light Attack")
        float LightAttackRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Light Attack")
        FVector LightAttackBoxSize = FVector(90.0f, 40.0f, 40.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Light Attack")
        float LightAttackDuration = 0.2f;

    // --- DASH ATTACK (Phase 2) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Dash Attack")
        float DashAttackDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Dash Attack")
        float DashAttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Dash Attack")
        FVector DashAttackBoxSize = FVector(100.0f, 50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Dash Attack")
        float DashAttackDuration = 0.5f;

    // --- HEAVY ATTACK (Phase 3) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Heavy Attack")
        float HeavyAttackDamage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Heavy Attack")
        float HeavyAttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Heavy Attack")
        FVector HeavyAttackBoxSize = FVector(120.0f, 60.0f, 60.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Combat | Heavy Attack")
        float HeavyAttackDuration = 0.6f;

    //loot
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        float DropChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        int32 MinLootCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        int32 MaxLootCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        TArray<FLootItem> LootTable;

    //vfx
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | VFX")
        UNiagaraSystem* AttackEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | VFX")
        UNiagaraSystem* HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | VFX")
        UNiagaraSystem* DeathEffect;

    //coins
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        TSubclassOf<ACPP_Item_Currency> CoinClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        int32 MinCoins = 5;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Loot")
        int32 MaxCoins = 15;
};

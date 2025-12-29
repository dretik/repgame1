// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseEnemy.h"
#include "CPP_BossEnemy.generated.h"

UENUM(BlueprintType)
enum class EBossAttackType : uint8
{
    Melee       UMETA(DisplayName = "Standing Melee"),
    Dash        UMETA(DisplayName = "Dash Attack"),
    Skyfall     UMETA(DisplayName = "Skyfall Ultimate")
};

USTRUCT(BlueprintType)
struct FBossAttackConfig
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere)
        EBossAttackType AttackType = EBossAttackType::Melee;

    UPROPERTY(EditAnywhere)
        UPaperFlipbook* AttackAnim = nullptr;

    UPROPERTY(EditAnywhere)
        UPaperFlipbook* AttackAnimLoop = nullptr;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.1"))
        float Weight = 1.0f;

    UPROPERTY(EditAnywhere)
        float Duration = 1.0f;

    UPROPERTY(EditAnywhere)
        float Damage = 20.0f;
};

UCLASS()
class PROJBLANK_API ACPP_BossEnemy : public ACPP_BaseEnemy
{
	GENERATED_BODY()
	
public:
    ACPP_BossEnemy(const FObjectInitializer& ObjectInitializer);

    virtual void Tick(float DeltaTime) override;

    virtual void AttackPlayer() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss | Config")
        TArray<FBossAttackConfig> AttackConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss | Skyfall")
        TSubclassOf<class ACPP_Projectile> SkyfallProjectileClass;

    UPaperFlipbook* CurrentLoopAnim;

    void ExecuteAttack(const FBossAttackConfig& Config);

    void DoMeleeAttack(float Damage);
    void DoDashAttack(float Damage);
    void StartSkyfall();

    UFUNCTION()
        void OnSkyfallIntroFinished(float LoopDuration);

    void SpawnSingleMeteor();

    virtual void FinishAttack() override;

    FTimerHandle SkyfallTimer;
    int32 MeteorsToSpawn = 0;

    float CurrentSkyfallDamage = 20.0f;
};

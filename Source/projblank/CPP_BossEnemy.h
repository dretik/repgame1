// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseEnemy.h"
#include "CPP_BossEnemy.generated.h"

USTRUCT(BlueprintType)
struct FBossAttackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        FGameplayTag ActionTag;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.1"))
        float Weight = 1.0f;
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
};

// Fill out your copyright notice in the Description page of Project Settings.

// CPP_BaseEnemy.h

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CPP_BaseEnemy.generated.h"

class UPawnSensingComponent;

UCLASS()
class PROJBLANK_API ACPP_BaseEnemy : public ACPP_BaseCharacter
{
    GENERATED_BODY()

public:
    ACPP_BaseEnemy(const FObjectInitializer& ObjectInitializer);

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "AI | Attack")
        virtual void AttackPlayer();

    UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetEnemyDamageMultiplier() const { return EnemyLevelDamageMultiplier; }

    bool bIsDynamicallySpawned = false;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
        UPawnSensingComponent* PawnSensingComp;

    FTimerHandle AttackCooldownTimer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* EnemyAttackFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
        float StoppingDistance = 80.0f;

    virtual void OnDeath_Implementation() override;

    void SpawnLoot();
    void SpawnCoins();
    void SpawnXP();

    float EnemyLevelDamageMultiplier = 1.0f;

    virtual void FinishAttack();

    void PerformHitTrace();

private:
    UFUNCTION()
        void OnPawnSeen(APawn* SeenPawn);

    virtual void BeginPlay() override;
    virtual bool CanDealDamageTo(AActor* TargetActor) const override;

};
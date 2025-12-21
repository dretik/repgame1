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
        void AttackPlayer();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
        UPawnSensingComponent* PawnSensingComp;

    FTimerHandle AttackCooldownTimer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* EnemyAttackFlipbook;

    virtual void OnDeath_Implementation() override;

    void SpawnLoot();

private:
    UFUNCTION()
        void OnPawnSeen(APawn* SeenPawn);

    virtual void BeginPlay() override;
    virtual bool CanDealDamageTo(AActor* TargetActor) const override;

    void PerformHitTrace();
    void FinishAttack();
};
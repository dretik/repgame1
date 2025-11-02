// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseCharacter.h" 
#include "CPP_BaseEnemy.generated.h"

class UPawnSensingComponent;

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_BaseEnemy : public ACPP_BaseCharacter
{
	GENERATED_BODY()
	
public:
    ACPP_BaseEnemy(const FObjectInitializer& ObjectInitializer);
    virtual void Tick(float DeltaTime) override;

protected:
    // Наш компонент "зрения"
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
        UPawnSensingComponent* PawnSensingComp;

private:
    // Функция, которая будет вызываться, когда компонент "увидит" пешку
    UFUNCTION()
        void OnPawnSeen(APawn* SeenPawn);
    virtual void BeginPlay() override;
};

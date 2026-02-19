// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_BossDashAttack.generated.h"

class UPaperFlipbook;

UCLASS()
class PROJBLANK_API UCPP_Action_BossDashAttack : public UCPP_Action
{
	GENERATED_BODY()
	
public:
	UCPP_Action_BossDashAttack();

	virtual void StartAction_Implementation(AActor* Instigator) override;
	virtual void StopAction_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float HitDelay = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		UPaperFlipbook* DashAnim;

	FTimerHandle TimerHandle_Hit;
	FTimerHandle TimerHandle_Duration;

	UFUNCTION()
		void MakeDashHit(AActor* Instigator);
};

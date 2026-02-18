// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_BossMelee.generated.h"

class UPaperFlipbook;

UCLASS()
class PROJBLANK_API UCPP_Action_BossMelee : public UCPP_Action
{
	GENERATED_BODY()
	
public:
	UCPP_Action_BossMelee();

	virtual void StartAction_Implementation(AActor* Instigator) override;
	virtual void StopAction_Implementation(AActor* Instigator) override;

protected:
	// Настройки урона
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float DamageAmount = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		FVector AttackBoxSize = FVector(200.f, 80.f, 80.f);

	// Тайминги
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float AttackDelay = 0.4f;

		// Анимация
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		UPaperFlipbook* AttackAnim;

	FTimerHandle TimerHandle_AttackDelay;
	FTimerHandle TimerHandle_Duration;

	UFUNCTION()
		void MakeHit(AActor* Instigator);
};

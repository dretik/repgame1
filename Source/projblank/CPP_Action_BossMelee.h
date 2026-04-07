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
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float AttackDelay = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float AttackRange = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		FVector AttackBoxSize = FVector(90.0f, 40.0f, 40.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float ActionDamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Movement")
		float DashImpulse = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Movement")
		FVector DashDirection = FVector(0.0f, 1.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Movement")
		float WalkSpeedModifier = -1.0f;

	float OriginalWalkSpeed = -1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		UPaperFlipbook* AttackAnim;

	FTimerHandle TimerHandle_AttackDelay;
	FTimerHandle TimerHandle_Duration;

	UFUNCTION()
		void MakeHit(AActor* Instigator);
};

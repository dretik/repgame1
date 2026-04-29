// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_Explosion.generated.h"

class UNiagaraSystem;
class UCPP_Action_Effect;

UCLASS()
class PROJBLANK_API UCPP_Action_Explosion : public UCPP_Action
{
	GENERATED_BODY()

public:
    UCPP_Action_Explosion();

    virtual void StartAction_Implementation(AActor* Instigator) override;

    virtual float GetActionDamageMultiplier() const override;

    virtual FText GetFormattedDescription_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Explosion | Config")
		float ExplosionRadius = 350.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion | Config")
		float PushForce = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion | Config")
		TArray<float> MultiplierPerLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion | Config")
		TArray<TSubclassOf<UCPP_Action_Effect>> StatusEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		UNiagaraSystem* ExplosionVFX;
};

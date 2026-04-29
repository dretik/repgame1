// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_Effect.generated.h"

UCLASS()
class PROJBLANK_API UCPP_Action_Effect : public UCPP_Action
{
	GENERATED_BODY()
	
public:
	UCPP_Action_Effect();

	virtual void StartAction_Implementation(AActor* Instigator) override;
	virtual void StopAction_Implementation(AActor* Instigator) override;
	virtual void RefreshAction_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
		float TickInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
		float DamagePerTick;

	UPROPERTY(EditDefaultsOnly, Category = "Effect | Stats")
		TArray<FStatModifier> ModifiersWhileActive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect | Stacking")
		bool bCanStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect | Stacking", meta = (EditCondition = "bCanStack"))
		int32 MaxStacks;

	UPROPERTY(BlueprintReadOnly, Category = "Effect | Stacking")
		int32 CurrentStacks;

	UPROPERTY(EditDefaultsOnly, Category = "Effect | Visuals")
		FLinearColor EffectColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, Category = "Effect | Visuals")
		float EmissiveBoost = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Effect | Visuals")
		float StatusIntensity = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Effect | Visuals")
		UNiagaraSystem* PersistentFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effect | Movement")
		float SpeedMultiplier = 1.0f;

	float SpeedModDelta;
	
	UPROPERTY()
		class UNiagaraComponent* ActiveFXComp;

	UFUNCTION()
		virtual void ExecuteTick();

	FTimerHandle TimerHandle_EffectTick;
};

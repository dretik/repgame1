// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_AreaPulse.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API UCPP_Action_AreaPulse : public UCPP_Action
{
	GENERATED_BODY()

public:
	UCPP_Action_AreaPulse();
	virtual void StartAction_Implementation(AActor* Instigator) override;
	virtual void StopAction_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Pulse Config")
		float Radius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pulse Config")
		float PulseInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		class UNiagaraSystem* PersistentVFX;

	UPROPERTY()
		class UNiagaraComponent* SpawnedVFX;

	virtual void OnPulse(const TArray<AActor*>& OverlappedActors, AActor* Instigator) {};

	UFUNCTION()
		void ExecutePulse(AActor* Instigator);

	FTimerHandle TimerHandle_Pulse;
};

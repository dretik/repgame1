// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action_AreaPulse.h"
#include "ActionTypes.h"
#include "CPP_Action_AuraField.generated.h"

UCLASS()
class PROJBLANK_API UCPP_Action_AuraField : public UCPP_Action_AreaPulse
{
	GENERATED_BODY()
public:
	UCPP_Action_AuraField();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Field | Physics")
		EFieldForceType ForceType = EFieldForceType::None;

	UPROPERTY(EditDefaultsOnly, Category = "Field | Physics")
		float ForceStrength = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Field | Physics")
		FVector WindDirection = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Field | Effects")
		TArray<TSubclassOf<class UCPP_Action_Effect>> EffectsToApply;

	// Основная логика тика поля
	virtual void OnPulse(const TArray<AActor*>& OverlappedActors, AActor* Instigator) override;
};

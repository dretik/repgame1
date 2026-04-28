// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Projectile.h"
#include "CPP_Projectile_Card.generated.h"

UCLASS()
class PROJBLANK_API ACPP_Projectile_Card : public ACPP_Projectile
{
	GENERATED_BODY()

public:
	//from card
	void SetImpactAction(TSubclassOf<class UCPP_Action> Action) { ImpactAction = Action; }

protected:
	UPROPERTY()
		TSubclassOf<class UCPP_Action> ImpactAction;

	//override explode to action 
	virtual void Explode(AActor* Target) override;
};

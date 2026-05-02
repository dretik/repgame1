// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action_AreaPulse.h"
#include "CPP_Action_Vortex.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API UCPP_Action_Vortex : public UCPP_Action_AreaPulse
{
	GENERATED_BODY()
	
protected:
	virtual void OnPulse(const TArray<AActor*>& OverlappedActors, AActor* Instigator) override;


};

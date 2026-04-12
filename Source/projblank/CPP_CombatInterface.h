// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPP_CombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCPP_CombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJBLANK_API ICPP_CombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool CanDealDamageTo(AActor* TargetActor) const = 0;
};

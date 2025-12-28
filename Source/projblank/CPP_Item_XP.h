// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Item_Currency.h"
#include "CPP_Item_XP.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_Item_XP : public ACPP_Item_Currency
{
	GENERATED_BODY()
	
public:
	ACPP_Item_XP();
	virtual void Interact_Implementation(AActor* Interactor) override;
};

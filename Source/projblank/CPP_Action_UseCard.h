// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "ActionTypes.h"
#include "CPP_Action_UseCard.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API UCPP_Action_UseCard : public UCPP_Action
{
	GENERATED_BODY()

public:
	UCPP_Action_UseCard();

	virtual void StartAction_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Card")
		ECardDrawMode DrawMode = ECardDrawMode::DrawTop;

	UPROPERTY(EditDefaultsOnly, Category = "Card")
		bool bSelfCastMode = false;
};

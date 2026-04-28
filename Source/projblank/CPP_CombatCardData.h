// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActionTypes.h"
#include "CPP_CombatCardData.generated.h"

UCLASS()
class PROJBLANK_API UCPP_CombatCardData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Definition")
		FCombatCard CardInfo;
};

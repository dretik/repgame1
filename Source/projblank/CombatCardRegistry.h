// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CPP_CombatCardData.h"
#include "CombatCardRegistry.generated.h"

UCLASS()
class PROJBLANK_API UCombatCardRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
		TArray<UCPP_CombatCardData*> AllCards;

	UFUNCTION(BlueprintCallable, Category = "Registry")
		UCPP_CombatCardData* GetRandomCard() const;
};

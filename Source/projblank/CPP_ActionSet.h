// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CPP_ActionSet.generated.h"

class UCPP_Action;

UCLASS()
class PROJBLANK_API UCPP_ActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Actions")
		TArray<TSubclassOf<UCPP_Action>> Actions;
};

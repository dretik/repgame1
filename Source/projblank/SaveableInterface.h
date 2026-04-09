// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableInterface.generated.h"

// ќЅя«ј“≈Ћ№Ќќ: говорим компил€тору, что такой класс существует
class UCPP_SaveGame;

UINTERFACE(MinimalAPI)
class USaveableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJBLANK_API ISaveableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveSystem")
		void OnSaveGame(UCPP_SaveGame* SaveObject);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveSystem")
		void OnLoadGame(UCPP_SaveGame* SaveObject);
};

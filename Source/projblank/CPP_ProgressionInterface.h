// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPP_BaseCharacter.h"
#include "CPP_ProgressionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCPP_ProgressionInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJBLANK_API ICPP_ProgressionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Progression")
        void ModifyStat(FStatModifier Modifier);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Progression")
        void AddResource(FGameplayTag ResourceTag, float Amount);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPP_DamageableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCPP_DamageableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJBLANK_API ICPP_DamageableInterface
{
    GENERATED_BODY()

public:
    //allows every object to say that it has health
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
        class UCPP_AttributeComponent* GetAttributeComponent() const;

    //hit reaction (could be sound)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
        void OnImpact(FVector ImpactLocation, AActor* DamageCauser);
};

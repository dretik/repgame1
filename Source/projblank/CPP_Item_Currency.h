// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseItem.h"
#include "CPP_Item_Currency.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_Item_Currency : public ACPP_BaseItem
{
	GENERATED_BODY()
	
public:
    ACPP_Item_Currency();

    // Функция для установки стоимости (вызывается врагом при спавне)
    void SetValue(int32 NewValue);

    virtual void Interact_Implementation(AActor* Interactor) override;

protected:
    // Номинал монетки (по умолчанию 1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
        int32 CoinValue = 1;
};

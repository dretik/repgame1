// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseItem.h"
#include "CPP_Item_Thief.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_Item_Thief : public ACPP_BaseItem
{
	GENERATED_BODY()
	
public:
	ACPP_Item_Thief();

	virtual void Interact_Implementation(AActor* Interactor) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thief Settings")
		TSubclassOf<ACPP_BaseItem> ItemToRemove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thief Settings")
		int32 AmountToRemove = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thief Settings")
		FText SuccessMessage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thief Settings")
		FText FailMessage;

};

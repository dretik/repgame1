// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseItem.h"
#include "CPP_CombatCardData.h"
#include "CPP_Item_CombatCard.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_Item_CombatCard : public ACPP_BaseItem
{
	GENERATED_BODY()
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;

	//allows to externally set up a card
	UFUNCTION(BlueprintCallable, Category = "Card")
		void InitializeCard(UCPP_CombatCardData* NewCardAsset);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
		class UCPP_CombatCardData* CardDataAsset;

	void BeginPlay();
};

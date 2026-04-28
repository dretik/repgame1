// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionTypes.h"
#include "CPP_DeckComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJBLANK_API UCPP_DeckComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPP_DeckComponent();

	UFUNCTION(BlueprintCallable, Category = "Deck")
		void AddCardToDeck(const FCombatCard& NewCard);

	//draw for use
	UFUNCTION(BlueprintCallable, Category = "Deck")
		bool DrawCard(ECardDrawMode DrawMode, FCombatCard& OutCard);

	//for ui in inventory
	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		void SwapCards(int32 IndexA, int32 IndexB);

	//card amount
	UFUNCTION(BlueprintCallable, Category = "Deck")
		int32 GetDeckCount() const { return DrawPile.Num(); }

	//for ui display
	UFUNCTION(BlueprintCallable, Category = "Deck")
		const TArray<FCombatCard>& GetAllCards() const { return DrawPile; }

	//delete card by index
	UFUNCTION(BlueprintCallable, Category = "Deck")
		void RemoveCard(int32 Index);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
		TArray<FCombatCard> DrawPile;

};
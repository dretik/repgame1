// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionTypes.h"
#include "CPP_DeckComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeckChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJBLANK_API UCPP_DeckComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPP_DeckComponent();

	UPROPERTY(BlueprintAssignable, Category = "Deck|Events")
		FOnDeckChanged OnDeckChanged;

	UFUNCTION(BlueprintCallable, Category = "Deck")
		void AddCard(const FCombatCard& NewCard);

	//draw for use
	UFUNCTION(BlueprintCallable, Category = "Deck")
		bool DrawCard(ECardDrawMode DrawMode, FCombatCard& OutCard);

	//for ui in inventory
	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		void SwapCards(int32 IndexA, int32 IndexB);
	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		void SwapCardWithInventory(int32 DeckIndex, int32 CardIndexInDeck, int32 InventoryIndex);

	////card amount
	//UFUNCTION(BlueprintCallable, Category = "Deck")
	//	int32 GetDeckCount() const { return DrawPile.Num(); }

	////for ui display
	//UFUNCTION(BlueprintCallable, Category = "Deck")
	//	const TArray<FCombatCard>& GetAllCards() const { return DrawPile; }

	////delete card by index
	//UFUNCTION(BlueprintCallable, Category = "Deck")
	//	void RemoveCard(int32 Index);

	//UI (DRAG & DROP)
	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		void SetActiveDeckIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		void MoveCardFromInventoryToDeck(int32 InventoryIndex, int32 TargetDeckIndex);

	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		void MoveCardFromDeckToInventory(int32 DeckIndex, int32 CardIndexInDeck);

	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		const TArray<FCombatCard>& GetInventoryCards() const { return CardInventory; }

	UFUNCTION(BlueprintCallable, Category = "Deck|UI")
		const FCardDeck& GetDeck(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Deck")
		int32 GetActiveDeckIndex() const { return ActiveDeckIndex; }

protected:
	virtual void BeginPlay() override;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
	//	TArray<FCombatCard> DrawPile;

	UPROPERTY(EditDefaultsOnly, Category = "Deck Config")
		int32 MaxDeckSize = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Deck Config")
		int32 TotalDecks = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck State")
		int32 ActiveDeckIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck State")
		TArray<FCardDeck> Decks;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck State")
		TArray<FCombatCard> CardInventory;
};
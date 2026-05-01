#include "CPP_DeckComponent.h"

UCPP_DeckComponent::UCPP_DeckComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPP_DeckComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int32 i = 0; i < TotalDecks; ++i)
	{
		FCardDeck NewDeck;
		NewDeck.DeckName = FText::FromString(FString::Printf(TEXT("Deck %d"), i + 1));
		Decks.Add(NewDeck);
	}
}

void UCPP_DeckComponent::AddCard(const FCombatCard& NewCard)
{
	if (!Decks.IsValidIndex(ActiveDeckIndex)) return;

	if (Decks[ActiveDeckIndex].Cards.Num() < MaxDeckSize)
	{
		Decks[ActiveDeckIndex].Cards.Add(NewCard);
	}
	else
	{
		CardInventory.Add(NewCard);
	}

	OnDeckChanged.Broadcast();

	//DrawPile.Add(NewCard);
}

bool UCPP_DeckComponent::DrawCard(ECardDrawMode DrawMode, FCombatCard& OutCard)
{
	if (!Decks.IsValidIndex(ActiveDeckIndex)) return false;

	TArray<FCombatCard>& ActiveCards = Decks[ActiveDeckIndex].Cards;

	if (ActiveCards.Num() == 0) return false;

	int32 DrawnIndex = 0;
	if (DrawMode == ECardDrawMode::DrawRandom)
	{
		DrawnIndex = FMath::RandRange(0, ActiveCards.Num() - 1);
	}

	OutCard = ActiveCards[DrawnIndex];
	ActiveCards.RemoveAt(DrawnIndex);

	OnDeckChanged.Broadcast();

	return true;
}

void UCPP_DeckComponent::SetActiveDeckIndex(int32 NewIndex)
{
	if (Decks.IsValidIndex(NewIndex))
	{
		ActiveDeckIndex = NewIndex;
		OnDeckChanged.Broadcast();
	}
}

void UCPP_DeckComponent::MoveCardFromInventoryToDeck(int32 InventoryIndex, int32 TargetDeckIndex)
{
	if (CardInventory.IsValidIndex(InventoryIndex) && Decks.IsValidIndex(TargetDeckIndex))
	{
		if (Decks[TargetDeckIndex].Cards.Num() < MaxDeckSize)
		{
			FCombatCard CardToMove = CardInventory[InventoryIndex];
			CardInventory.RemoveAt(InventoryIndex);
			Decks[TargetDeckIndex].Cards.Add(CardToMove);

			OnDeckChanged.Broadcast();
		}
	}
}

void UCPP_DeckComponent::MoveCardFromDeckToInventory(int32 DeckIndex, int32 CardIndexInDeck)
{
	if (Decks.IsValidIndex(DeckIndex) && Decks[DeckIndex].Cards.IsValidIndex(CardIndexInDeck))
	{
		FCombatCard CardToMove = Decks[DeckIndex].Cards[CardIndexInDeck];
		Decks[DeckIndex].Cards.RemoveAt(CardIndexInDeck);
		CardInventory.Add(CardToMove);

		OnDeckChanged.Broadcast();
	}
}

const FCardDeck& UCPP_DeckComponent::GetDeck(int32 Index) const
{
	if (Decks.IsValidIndex(Index)) return Decks[Index];

	// Fallback если индекс неверный
	static FCardDeck EmptyDeck;
	return EmptyDeck;
}

void UCPP_DeckComponent::SwapCards(int32 IndexA, int32 IndexB)
{
	if (Decks.IsValidIndex(ActiveDeckIndex))
	{
		TArray<FCombatCard>& TargetCards = Decks[ActiveDeckIndex].Cards;
		if (TargetCards.IsValidIndex(IndexA) && TargetCards.IsValidIndex(IndexB))
		{
			TargetCards.Swap(IndexA, IndexB);
			OnDeckChanged.Broadcast();
		}
	}
}

void UCPP_DeckComponent::SwapCardWithInventory(int32 DeckIndex, int32 CardIndexInDeck, int32 InventoryIndex)
{
	if (Decks.IsValidIndex(DeckIndex) &&
		Decks[DeckIndex].Cards.IsValidIndex(CardIndexInDeck) &&
		CardInventory.IsValidIndex(InventoryIndex))
	{
		FCombatCard CardFromDeck = Decks[DeckIndex].Cards[CardIndexInDeck];

		Decks[DeckIndex].Cards[CardIndexInDeck] = CardInventory[InventoryIndex];
		CardInventory[InventoryIndex] = CardFromDeck;

		OnDeckChanged.Broadcast();
	}
}
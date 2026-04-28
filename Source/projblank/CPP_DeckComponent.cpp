#include "CPP_DeckComponent.h"

UCPP_DeckComponent::UCPP_DeckComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPP_DeckComponent::AddCardToDeck(const FCombatCard& NewCard)
{
	DrawPile.Add(NewCard);
}

bool UCPP_DeckComponent::DrawCard(ECardDrawMode DrawMode, FCombatCard& OutCard)
{
	if (DrawPile.Num() == 0) return false;

	int32 DrawnIndex = 0;
	if (DrawMode == ECardDrawMode::DrawRandom)
	{
		DrawnIndex = FMath::RandRange(0, DrawPile.Num() - 1);
	}

	OutCard = DrawPile[DrawnIndex];
	DrawPile.RemoveAt(DrawnIndex);

	return true;
}

void UCPP_DeckComponent::SwapCards(int32 IndexA, int32 IndexB)
{
	if (DrawPile.IsValidIndex(IndexA) && DrawPile.IsValidIndex(IndexB))
	{
		DrawPile.Swap(IndexA, IndexB);
	}
}

void UCPP_DeckComponent::RemoveCard(int32 Index)
{
	if (DrawPile.IsValidIndex(Index))
	{
		DrawPile.RemoveAt(Index);
	}
}
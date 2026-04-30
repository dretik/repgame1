// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Item_CombatCard.h"
#include "CPP_DeckComponent.h"

void ACPP_Item_CombatCard::Interact_Implementation(AActor* Interactor)
{
	if (!CardDataAsset) return;

	UCPP_DeckComponent* DeckComp = Interactor->FindComponentByClass<UCPP_DeckComponent>();
	if (DeckComp)
	{
		DeckComp->AddCard(CardDataAsset->CardInfo);


		// notif, delete
		Super::Interact_Implementation(Interactor);
	}
}

void ACPP_Item_CombatCard::BeginPlay()
{
	if (CardDataAsset)
	{
		InitializeCard(CardDataAsset);
	}

	Super::BeginPlay();
}

void ACPP_Item_CombatCard::InitializeCard(UCPP_CombatCardData* NewCardAsset)
{
	if (!NewCardAsset) return;

	CardDataAsset = NewCardAsset;
	ItemName = CardDataAsset->CardInfo.CardName;

	if (ItemSprite && CardDataAsset->CardInfo.CardIcon)
	{

	}
}
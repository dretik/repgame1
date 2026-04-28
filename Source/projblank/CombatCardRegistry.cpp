// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCardRegistry.h"

UCPP_CombatCardData* UCombatCardRegistry::GetRandomCard() const
{
	if (AllCards.Num() == 0) return nullptr;

	float TotalWeight = 0.0f;
	for (UCPP_CombatCardData* Card : AllCards)
	{
		if (Card) TotalWeight += Card->CardInfo.DropWeight;
	}

	float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentSum = 0.0f;

	for (UCPP_CombatCardData* Card : AllCards)
	{
		if (!Card) continue;
		CurrentSum += Card->CardInfo.DropWeight;
		if (RandomPoint <= CurrentSum)
		{
			return Card;
		}
	}
	return AllCards[0];
}

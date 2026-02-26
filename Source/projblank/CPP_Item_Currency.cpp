// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Item_Currency.h"
#include "CPP_BaseCharacter.h"
#include "CPP_PlayerCharacter.h"

ACPP_Item_Currency::ACPP_Item_Currency()
{
    bIsInventoryItem = false;
    bAutoPickup = true;
    ItemName = NSLOCTEXT("Currency", "CoinName", "Gold Coin");
}

void ACPP_Item_Currency::SetValue(int32 NewValue)
{
    CoinValue = NewValue;
}

void ACPP_Item_Currency::Interact_Implementation(AActor* Interactor)
{
    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(Interactor);
    if (Player)
    {
        Player->AddCoins(CoinValue);

        FText MessagePattern = NSLOCTEXT("Currency", "PickupMessage", "Picked up: {0} coins");

        FText FinalMessage = FText::Format(
            MessagePattern,
            FText::AsNumber(CoinValue)
        );
        Player->ShowNotification(FinalMessage, FColor::Yellow);

        Destroy();
    }
}
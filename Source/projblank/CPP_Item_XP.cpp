// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Item_XP.h"
#include "CPP_BaseCharacter.h"
#include "CPP_PlayerCharacter.h"

ACPP_Item_XP::ACPP_Item_XP()
{
    ItemName = NSLOCTEXT("Items", "XPName", "Experience Orb");
    CoinValue = 20;
}

void ACPP_Item_XP::Interact_Implementation(AActor* Interactor)
{
    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(Interactor);
    if (Player)
    {
        Player->AddExperience((float)CoinValue);

        FText Msg = FText::Format(NSLOCTEXT("HUD", "XPMsg", "+{0} XP"), FText::AsNumber(CoinValue));
        Player->ShowNotification(Msg, FColor::Purple);

        Destroy();
    }
}
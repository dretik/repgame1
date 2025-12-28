// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Item_Thief.h"
#include "CPP_BaseCharacter.h"
#include "CPP_InventoryComponent.h"
#include "CPP_Item_Currency.h"
#include "Engine/Engine.h"

ACPP_Item_Thief::ACPP_Item_Thief()
{
	bIsInventoryItem = false;
	SuccessMessage = NSLOCTEXT("Shop", "DefaultSuccess", "Payed items!");
	FailMessage = NSLOCTEXT("Shop", "DefaultFail", "Not enough items to pay!");
	ItemName = NSLOCTEXT("Items", "ThiefItemName", "Thief Item");
}

void ACPP_Item_Thief::Interact_Implementation(AActor* Interactor)
{
	ACPP_BaseCharacter* Player = Cast<ACPP_BaseCharacter>(Interactor);
	if (!Player) return;

	UCPP_InventoryComponent* InventoryComp = Player->FindComponentByClass<UCPP_InventoryComponent>();

	bool bSuccess = false;
	
	if (InventoryComp && ItemToRemove)
	{
		if (ItemToRemove->IsChildOf(ACPP_Item_Currency::StaticClass()))
		{
			if (Player->GetCoinCount() >= AmountToRemove)
			{
				Player->AddCoins(-AmountToRemove);
				bSuccess = true;
			}
		}
		else if (InventoryComp)
		{
			bSuccess = InventoryComp->RemoveItem(ItemToRemove, AmountToRemove);
		}

		if (bSuccess)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, SuccessMessage.ToString());
			}
			// soundcue can be played
			for (const FStatModifier& Mod : StatModifiers)
			{
				Player->ApplyStatModifier(Mod);
			}
			if (bIsInventoryItem && InventoryComp)
			{
				InventoryComp->AddItem(this->GetClass(), 1);
			}
			FText Msg = FText::Format(
				NSLOCTEXT("HUD", "PickupMessage", "Picked up: {0}"),
				ItemName
			);
			Player->ShowNotification(Msg, FColor::Yellow);

			Destroy();
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, FailMessage.ToString());
			}
			// if not enough items logic 
			// Player->TakeDamage(10.0f, ...); 
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Error: Inventory or ItemClass missing!"));
	}

}
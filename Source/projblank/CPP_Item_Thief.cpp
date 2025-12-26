// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Item_Thief.h"
#include "CPP_BaseCharacter.h"
#include "CPP_InventoryComponent.h"
#include "Engine/Engine.h"

ACPP_Item_Thief::ACPP_Item_Thief()
{
	bIsInventoryItem = false;
	ItemName = FName("Thief Item");
}

void ACPP_Item_Thief::Interact_Implementation(AActor* Interactor)
{
	ACPP_BaseCharacter* Player = Cast<ACPP_BaseCharacter>(Interactor);
	if (!Player) return;

	UCPP_InventoryComponent* InventoryComp = Player->FindComponentByClass<UCPP_InventoryComponent>();

	if (InventoryComp && ItemToRemove)
	{
		bool bSuccess = InventoryComp->RemoveItem(ItemToRemove, AmountToRemove);

		if (bSuccess)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, SuccessMessage);
			}
			// soundcue can be played
			for (const FStatModifier& Mod : StatModifiers)
			{
				Player->ApplyStatModifier(Mod);
			}
			if (bIsInventoryItem)
			{
				InventoryComp->AddItem(this->GetClass(), 1);
			}
			FString Msg = FString::Printf(TEXT("%s purchased!"), *ItemName.ToString());
			Player->ShowNotification(Msg, FColor::Yellow);

			Destroy();
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, FailMessage);
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
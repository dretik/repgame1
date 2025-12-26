// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_InventoryComponent.h"

// Sets default values for this component's properties
UCPP_InventoryComponent::UCPP_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

// Called when the game starts
void UCPP_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
}
void UCPP_InventoryComponent::AddItem(TSubclassOf<ACPP_BaseItem> ItemClass, int32 Amount)
{
    if (!ItemClass) return;

    bool bFound = false;

    for (FInventorySlot& Slot : Inventory)
    {
        if (Slot.ItemClass == ItemClass)
        {
            Slot.Count += Amount;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        FInventorySlot NewSlot;
        NewSlot.ItemClass = ItemClass;
        NewSlot.Count = Amount;
        Inventory.Add(NewSlot);
    }

    OnInventoryUpdated.Broadcast(Inventory);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Item Added to Inventory!"));
}

// Called every frame
void UCPP_InventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCPP_InventoryComponent::RemoveItem(TSubclassOf<ACPP_BaseItem> ItemClass, int32 Amount)
{
    if (!ItemClass || Amount <= 0) return false;

    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i].ItemClass == ItemClass)
        {
            if (Inventory[i].Count >= Amount)
            {
                Inventory[i].Count -= Amount;

                if (Inventory[i].Count <= 0)
                {
                    Inventory.RemoveAt(i);
                }

                OnInventoryUpdated.Broadcast(Inventory);

                return true; 
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

int32 UCPP_InventoryComponent::GetItemAmount(TSubclassOf<ACPP_BaseItem> ItemClass) const
{
    for (const FInventorySlot& Slot : Inventory)
    {
        if (Slot.ItemClass == ItemClass)
        {
            return Slot.Count;
        }
    }
    return 0;
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_BaseItem.h"
#include "CPP_InventoryComponent.generated.h"

//slot structure
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly)
		TSubclassOf<ACPP_BaseItem> ItemClass;

	UPROPERTY(BlueprintReadOnly)
		int32 Count = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const TArray<FInventorySlot>&, InventoryItems);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJBLANK_API UCPP_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPP_InventoryComponent();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
		TArray<FInventorySlot> Inventory;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void AddItem(TSubclassOf<ACPP_BaseItem> ItemClass, int32 Amount = 1);

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnInventoryUpdated OnInventoryUpdated;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

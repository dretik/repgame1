// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RarityTypes.h"
#include "ActionTypes.generated.h"

class UCPP_Action;
class UTexture2D;

UENUM(BlueprintType)
enum class ECardDrawMode : uint8
{
	DrawTop     UMETA(DisplayName = "deck top"),
	DrawRandom  UMETA(DisplayName = "deck random")
};

USTRUCT(BlueprintType)
struct FCombatCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
		FName CardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
		FText CardName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
		FText CardDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
		UTexture2D* CardIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ECardRarity Rarity=ECardRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FStatModifier> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Info")
		float DropWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Logic")
		TSubclassOf<class ACPP_Projectile_Card> CardProjectileClass;

	//projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Logic")
		float ThrowSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Logic")
		float CardCollisionRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Logic")
		float VisualScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Logic")
		TSubclassOf<class UCPP_Action> ImpactActionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Logic")
		TSubclassOf<class UCPP_Action> SelfActionClass;
};
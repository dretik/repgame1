// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CPP_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API UCPP_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveSystem")
        FString SaveSlotName = "SaveSlot01";

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        void CreateNewSave();

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        void SaveGame(class ACPP_BaseCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        bool LoadGame(class ACPP_BaseCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        bool HasSaveGame();

    UFUNCTION(BlueprintCallable, Category = "GameFlow")
        void LaunchNewGame(FName MapName);

    UPROPERTY(BlueprintReadWrite, Category = "SaveSystem")
        bool bIsLoadingSave = false;

    UFUNCTION(BlueprintCallable, Category = "GameFlow")
        bool ContinueGame();

    UPROPERTY()
        TSet<FString> CurrentSessionCollectedItems;

    void AddCollectedItem(FString ItemNameID) { CurrentSessionCollectedItems.Add(ItemNameID); }

    void RespawnDynamicEnemies(UWorld* World);
};

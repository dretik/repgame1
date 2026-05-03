// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RunTypes.h"
#include "CPP_GameInstance.generated.h"

class UCCP_Action;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldLevelUp, int32, NewLevel);


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
        void SaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        bool LoadGame();

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        bool HasSaveGame();

    UFUNCTION(BlueprintCallable, Category = "GameFlow")
        void LaunchNewGame(FName MapName);

    UPROPERTY(BlueprintReadWrite, Category = "SaveSystem")
        bool bIsLoadingSave = false;

    UFUNCTION(BlueprintCallable, Category = "GameFlow")
        bool ContinueGame();

    UFUNCTION(BlueprintCallable, Category = "GameFlow")
        void ReturnToMenu();

    UFUNCTION(BlueprintCallable, Category = "GameFlow")
        void RestartRun();

    UPROPERTY()
        TSet<FString> CurrentSessionCollectedItems;

    UPROPERTY()
        TSet<FString> DestroyedStaticActors;

    void AddCollectedItem(FString ItemNameID) { CurrentSessionCollectedItems.Add(ItemNameID); }

    //Run manager config
    UPROPERTY(EditDefaultsOnly, Category = "Run Config|Tier 1")
        TArray<FLevelData> Pool_Tier1_Normal;

    UPROPERTY(EditDefaultsOnly, Category = "Run Config|Tier 1")
        TArray<FLevelData> Pool_Tier1_Boss;

    UPROPERTY(EditDefaultsOnly, Category = "Run Config")
        int32 LevelsBeforeBoss = 3;

    //Run manager state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Run State")
        TArray<FLevelData> CurrentRunRoute;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Run State")
        int32 CurrentLevelIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Run State")
        int32 CurrentRunScore = 0;

    //starter ability
    UPROPERTY(BlueprintReadWrite, Category = "Run State")
        TSubclassOf<UCPP_Action> SelectedStarterAbility;

    //Run manager flow
    UFUNCTION(BlueprintCallable, Category = "Run Flow")
        void StartNewRun();

    UFUNCTION(BlueprintCallable, Category = "Run Flow")
        void AdvanceToNextLevel();

    UFUNCTION(BlueprintCallable, Category = "Run Flow")
        void AddRunScore(int32 Points);

    //world level
    UPROPERTY(BlueprintAssignable, Category = "Progression")
        FOnWorldLevelUp OnWorldLevelUp;

    UFUNCTION(BlueprintCallable, Category = "Progression")
        void AddWorldExperience(float Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
        int32 GetWorldLevel() const { return WorldLevel; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        int32 WorldLevel = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        float CurrentWorldXP = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
        float WorldXPToNextLevel = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
        float WorldLevelMultiplier = 1.5f;
};
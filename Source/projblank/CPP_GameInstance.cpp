// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_GameInstance.h"
#include "CPP_SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "SaveableInterface.h"
#include "CPP_AttributeComponent.h"

void UCPP_GameInstance::CreateNewSave()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
    }
}

bool UCPP_GameInstance::HasSaveGame()
{
    return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

void UCPP_GameInstance::SaveGame()
{
    UCPP_SaveGame* SaveInst = Cast<UCPP_SaveGame>(UGameplayStatics::CreateSaveGameObject(UCPP_SaveGame::StaticClass()));

    // saving general data
    SaveInst->CollectedItems = this->CurrentSessionCollectedItems;

    for (const FString& DeadActorName : DestroyedStaticActors)
    {
        FEnemySaveData Data;
        Data.bIsDead = true;
        SaveInst->WorldEnemies.Add(DeadActorName, Data);
    }

    // gathering all actors, who do utilize ISaveableInterface
    TArray<AActor*> SaveableActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableInterface::StaticClass(), SaveableActors);

    for (AActor* Actor : SaveableActors)
    {
        // call to every actor to save
        ISaveableInterface::Execute_OnSaveGame(Actor, SaveInst);
    }

    UGameplayStatics::SaveGameToSlot(SaveInst, SaveSlotName, 0);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Modular Game Saved!"));
}

bool UCPP_GameInstance::LoadGame()
{
    if (!HasSaveGame()) return false;

    UCPP_SaveGame* LoadInst = Cast<UCPP_SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!LoadInst) return false;
    
    DestroyedStaticActors.Empty();

    for (auto& It : LoadInst->WorldEnemies)
    {
        if (It.Value.bIsDead)
        {
            DestroyedStaticActors.Add(It.Key);
        }
    }

    //global data
    this->CurrentSessionCollectedItems = LoadInst->CollectedItems;

    //dynamic actors
    for (const FDynamicEnemyData& Data : LoadInst->SpawnedEnemies)
    {
        if (Data.EnemyClass)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Data.EnemyClass, Data.Transform, Params);

            if (SpawnedActor)
            {
                SpawnedActor->Tags.Add(FName("Dynamic"));

                UCPP_AttributeComponent* AttrComp = SpawnedActor->FindComponentByClass<UCPP_AttributeComponent>();
                if (AttrComp)
                {
                    float MaxHP = (Data.MaxHealth > 0.0f) ? Data.MaxHealth : AttrComp->GetMaxHealth();
                    AttrComp->InitializeStats(MaxHP);

                    float Mult = (Data.SavedDamageMultiplier > 0.0f) ? Data.SavedDamageMultiplier : 1.0f;
                    AttrComp->SetDamageMultiplier(Mult);

                    float Diff = Data.CurrentHealth - AttrComp->GetHealth();
                    AttrComp->ApplyHealthChange(nullptr, Diff);
                }
            }
        }
    }

    // call to every actor to restore
    TArray<AActor*> SaveableActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableInterface::StaticClass(), SaveableActors);

    for (AActor* Actor : SaveableActors)
    {
        ISaveableInterface::Execute_OnLoadGame(Actor, LoadInst);
    }

    UGameplayStatics::SetGamePaused(this, false);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = false;
    }

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Modules Loaded!"));
    
    bIsLoadingSave = false;
    
    return true;
}

void UCPP_GameInstance::LaunchNewGame(FName MapName)
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        // UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
    }

    bIsLoadingSave = false;

    UGameplayStatics::SetGamePaused(this, false);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = false;
    }

    UGameplayStatics::OpenLevel(this, MapName);
}

bool UCPP_GameInstance::ContinueGame()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        bIsLoadingSave = true;

        UCPP_SaveGame* LoadInst = Cast<UCPP_SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

        FName MapToLoad = "Playground";
        if (LoadInst)
        {
            MapToLoad = LoadInst->LevelName;
        }

        UGameplayStatics::OpenLevel(this, MapToLoad);
        return true;
    }
    return false;
}

void UCPP_GameInstance::ReturnToMenu()
{
    //session clear
    CurrentSessionCollectedItems.Empty();
    DestroyedStaticActors.Empty();
    bIsLoadingSave = false;

    UGameplayStatics::SetGamePaused(this, false);

    UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UCPP_GameInstance::RestartRun()
{
    //current session clear
    CurrentSessionCollectedItems.Empty();
    DestroyedStaticActors.Empty();
    bIsLoadingSave = false;

    FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld()));

    UGameplayStatics::SetGamePaused(this, false);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = false;
    }

    UGameplayStatics::OpenLevel(this, CurrentLevelName);
}

void UCPP_GameInstance::StartNewRun()
{
    //default
    CurrentRunScore = 0;
    CurrentLevelIndex = 0;
    CurrentRunRoute.Empty();
    CurrentSessionCollectedItems.Empty();
    DestroyedStaticActors.Empty();
    bIsLoadingSave = false;

    TArray<FLevelData> TempPool = Pool_Tier1_Normal;

    for (int32 i = 0; i < LevelsBeforeBoss; ++i)
    {
        if (TempPool.Num() > 0)
        {
            int32 RandomIdx = FMath::RandRange(0, TempPool.Num() - 1);
            CurrentRunRoute.Add(TempPool[RandomIdx]);

            TempPool.RemoveAt(RandomIdx);
        }
    }

    if (Pool_Tier1_Boss.Num() > 0)
    {
        int32 BossIdx = FMath::RandRange(0, Pool_Tier1_Boss.Num() - 1);
        CurrentRunRoute.Add(Pool_Tier1_Boss[BossIdx]);
    }

    if (CurrentRunRoute.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to generate run route! Pools might be empty."));
        return;
    }

    AdvanceToNextLevel();
}

void UCPP_GameInstance::AdvanceToNextLevel()
{
    if (CurrentRunRoute.IsValidIndex(CurrentLevelIndex))
    {
        DestroyedStaticActors.Empty();

        UGameplayStatics::SetGamePaused(this, false);
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            FInputModeGameOnly Mode;
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = false;
        }

        FName LevelToLoad = CurrentRunRoute[CurrentLevelIndex].LevelName;

        CurrentLevelIndex++;

        UGameplayStatics::OpenLevel(this, LevelToLoad);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Run Completed!"));
        // victoru screen could be called (RunCompleteScreen) 
        // and return to main menu or hub
    }
}

void UCPP_GameInstance::AddRunScore(int32 Points)
{
    CurrentRunScore += Points;
}

void UCPP_GameInstance::AddWorldExperience(float Amount)
{
    float BalancedAmount = Amount * 0.75f;

    CurrentWorldXP += BalancedAmount;

    while (CurrentWorldXP >= WorldXPToNextLevel)
    {
        CurrentWorldXP -= WorldXPToNextLevel;
        WorldLevel++;
        WorldXPToNextLevel *= WorldLevelMultiplier;

        OnWorldLevelUp.Broadcast(WorldLevel);

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            FString::Printf(TEXT("World level increased to %d!"), WorldLevel));
    }
}
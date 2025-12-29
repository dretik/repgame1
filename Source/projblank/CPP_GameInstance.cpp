// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_GameInstance.h"
#include "CPP_SaveGame.h"
#include "CPP_BaseEnemy.h"
#include "CPP_BaseItem.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_BaseCharacter.h"
#include "CPP_InventoryComponent.h"
#include "CPP_BaseEnemy.h" 

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

void UCPP_GameInstance::SaveGame(ACPP_BaseCharacter* Player)
{
    if (!Player) return;

    UCPP_SaveGame* SaveInst = Cast<UCPP_SaveGame>(UGameplayStatics::CreateSaveGameObject(UCPP_SaveGame::StaticClass()));

    SaveInst->Health = Player->GetCurrentHealth();
    SaveInst->MaxHealth = Player->GetCurrentMaxHealth();
    SaveInst->BaseDamage = Player->GetCurrentBaseDamage(); 
    SaveInst->Level = Player->GetCharacterLevel();
    SaveInst->CurrentXP = Player->GetCurrentXP();
    SaveInst->Coins = Player->GetCoinCount();

    SaveInst->PlayerLocation = Player->GetActorLocation();
    SaveInst->LevelName = FName(*GetWorld()->GetName());

    UCPP_InventoryComponent* InvComp = Player->FindComponentByClass<UCPP_InventoryComponent>();
    if (InvComp)
    {
        SaveInst->InventoryData = InvComp->GetInventory(); 
    }

    SaveInst->AbilityLevels = Player->GetAbilityLevels(); 

    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_BaseEnemy::StaticClass(), FoundEnemies);

    for (AActor* Actor : FoundEnemies)
    {
        ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(Actor);
        if (!Enemy) continue;

        if (Enemy->IsDead())
        {
            if (Enemy->bIsDynamicallySpawned)
            {
                continue;
            }
            else
            {
                FEnemySaveData Data;
                Data.bIsDead = true;
                SaveInst->WorldEnemies.Add(Enemy->GetName(), Data);
            }
        }
        else
        {
            if (Enemy->bIsDynamicallySpawned)
            {
                FDynamicEnemyData DynData;
                DynData.EnemyClass = Enemy->GetClass();
                DynData.Transform = Enemy->GetActorTransform();
                DynData.CurrentHealth = Enemy->GetCurrentHealth();

                SaveInst->SpawnedEnemies.Add(DynData);
            }
            else
            {
                FEnemySaveData Data;
                Data.CurrentHealth = Enemy->GetCurrentHealth();
                Data.Location = Enemy->GetActorLocation();
                Data.bIsDead = false;

                SaveInst->WorldEnemies.Add(Enemy->GetName(), Data);
            }
        }
    }

    SaveInst->CollectedItems = this->CurrentSessionCollectedItems;

    UGameplayStatics::SaveGameToSlot(SaveInst, SaveSlotName, 0);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Game Saved!"));
}

bool UCPP_GameInstance::LoadGame(ACPP_BaseCharacter* Player)
{
    if (!Player) return false;
    if (!HasSaveGame()) return false;

    UCPP_SaveGame* LoadInst = Cast<UCPP_SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

    if (LoadInst)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
            FString::Printf(TEXT("Loading from file... HP: %f, Coins: %d"), LoadInst->Health, LoadInst->Coins));

        Player->SetStatsFromSave(
            LoadInst->Health,
            LoadInst->MaxHealth,
            LoadInst->BaseDamage,
            LoadInst->Level,
            LoadInst->CurrentXP,
            LoadInst->Coins
        );

        Player->SetLocationFromSave(LoadInst->PlayerLocation);

        UCPP_InventoryComponent* InvComp = Player->FindComponentByClass<UCPP_InventoryComponent>();
        if (InvComp)
        {
            InvComp->SetInventory(LoadInst->InventoryData);
        }

        Player->SetAbilityLevels(LoadInst->AbilityLevels);

        this->CurrentSessionCollectedItems = LoadInst->CollectedItems;

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Game Loaded!"));
        return true;
    }
    return false;
}

void UCPP_GameInstance::LaunchNewGame(FName MapName)
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        // UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
    }

    bIsLoadingSave = false;

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

void UCPP_GameInstance::RespawnDynamicEnemies(UWorld* World)
{
    if (!World) return;

    UCPP_SaveGame* LoadInst = Cast<UCPP_SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!LoadInst) return;

    for (const FDynamicEnemyData& Data : LoadInst->SpawnedEnemies)
    {
        if (Data.EnemyClass)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedActor = World->SpawnActor<AActor>(Data.EnemyClass, Data.Transform, Params);

            ACPP_BaseEnemy* Enemy = Cast<ACPP_BaseEnemy>(SpawnedActor);
            if (Enemy)
            {
                Enemy->bIsDynamicallySpawned = true;
                Enemy->SetCurrentHealth(Data.CurrentHealth); 
            }
        }
    }
}
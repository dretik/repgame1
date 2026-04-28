#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharacterStats.h"
#include "CPP_LootStatics.generated.h"

UCLASS()
class PROJBLANK_API UCPP_LootStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // spawns loot coins xp according to CharacterStats
    UFUNCTION(BlueprintCallable, Category = "Loot", meta = (WorldContext = "WorldContextObject"))
        static void SpawnAllLoot(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location);

    //could be private if dont need separate loot logic
    static void ExecuteSpawnLootTable(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location);
    static void ExecuteSpawnCurrency(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location);
    static void ExecuteSpawnXP(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location);
    static void ExecuteSpawnCard(const UObject* WorldContextObject, UCharacterStats* Stats, FVector Location);
private:

    static void ApplyLootImpulse(AActor* Item);
};
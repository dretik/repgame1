// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseCharacter.h"

#include "InteractableInterface.h"
#include "CPP_ProgressionInterface.h"

#include "CPP_PlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNotificationReceived, FText, Message, FLinearColor, Color);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinsUpdated, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnXPUpdated, float, CurrentXP, float, MaxXP, int32, Level);

UCLASS()
class PROJBLANK_API ACPP_PlayerCharacter : public ACPP_BaseCharacter, public ICPP_ProgressionInterface
{
    GENERATED_BODY()

public:
    ACPP_PlayerCharacter(const FObjectInitializer& ObjectInitializer);

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void OnJumped_Implementation() override;

    // --- INTERACTION ---
    void InteractWithObject();

    // --- UI DELEGATES ---
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnNotificationReceived OnNotificationReceived;
    void ShowNotification(FText Text, FLinearColor Color = FLinearColor::White);

    // --- ABILITIES ---
    UFUNCTION(BlueprintCallable, Category = "Abilities")
        bool GrantAbility(TSubclassOf<class UCPP_Action> ActionClass);
    //UFUNCTION(BlueprintCallable, Category = "Abilities")
    //    bool HasAbility(FGameplayTag AbilityTag) const;
    //UFUNCTION(BlueprintCallable, Category = "Abilities")
    //    int32 GetAbilityLevel(FGameplayTag AbilityTag) const;
    //const TMap<FGameplayTag, int32>& GetAbilityLevels() const { return AbilityLevels; }
    //void SetAbilityLevels(const TMap<FGameplayTag, int32>& LoadedAbilities);

    // --- ECONOMY ---
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnCoinsUpdated OnCoinsUpdated;
    UFUNCTION(BlueprintCallable, Category = "Economy")
        void AddCoins(int32 Amount);
    int32 GetCoinCount() const { return CoinCount; }

    // --- PROGRESSION ---
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnXPUpdated OnXPUpdated;
    UFUNCTION(BlueprintCallable, Category = "Progression")
        int32 GetCharacterLevel() const { return CharacterLevel; }
    UFUNCTION(BlueprintCallable, Category = "Progression")
        void AddExperience(float Amount);
    UFUNCTION(BlueprintCallable, Category = "Progression")
        float GetCurrentXP() const { return CurrentXP; }
    UFUNCTION(BlueprintCallable, Category = "Progression")
        void RemoveExperience(float Amount);

    virtual void ModifyStat_Implementation(FStatModifier Modifier) override;
    virtual void AddResource_Implementation(FGameplayTag ResourceTag, float Amount) override;

    // --- SAVES ---
    //UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    //    void SetStatsFromSave(float SavedHealth, float SavedMaxHealth, float SavedBaseDamage, int32 SavedLevel, float SavedXP, int32 SavedCoins);
    // ISaveableInterface
    virtual void OnSaveGame_Implementation(UCPP_SaveGame* SaveObject) override;
    virtual void OnLoadGame_Implementation(UCPP_SaveGame* SaveObject) override;

    // virtual for gold
    virtual void ApplyStatModifier(FStatModifier Modifier);

protected:
    virtual void BeginPlay() override;

    // --- INPUT ACTIONS ---
    void MoveRight(float Value);
    void MoveForward(float Value);
    void Dodge();
    void StopDodge();
    void StopJump();

    UFUNCTION(BlueprintCallable, Category = "Actions")
        void PrimaryAttack();
    UFUNCTION(BlueprintCallable, Category = "Actions")
        void MagicAttack();

    void InputAbilityQ();
    void InputAbilityE();
    void InputAbilityR();

    // --- COMBO SYSTEM ---
    bool bInputBuffered = false;
    void ExecuteMeleeAttack();
    void ResetCombo();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State | Attack")
        int32 ComboCounter = 0;

    FTimerHandle ComboResetTimer;

    UFUNCTION()
        void OnAttackActionStopped(class UCPP_Action* Action);

    // --- DATA ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
        int32 CoinCount = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
        float CoinMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        int32 CharacterLevel = 1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        float CurrentXP = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        float XPToNextLevel = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
        float LevelUpMultiplier = 1.2f;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    //    TMap<FGameplayTag, int32> AbilityLevels;

    void LevelUp();
};
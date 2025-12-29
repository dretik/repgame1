// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterStats.h"
#include "InteractableInterface.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "CPP_BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, MaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNotificationReceived, FText, Message, FLinearColor, Color);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinsUpdated, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnXPUpdated, float, CurrentXP, float, MaxXP, int32, Level);

class UPaperFlipbook;

UENUM(BlueprintType)
enum class EAttackPhase : uint8
{
    None = 0 UMETA(DisplayName = "No Attack"),
    LightAttack = 1 UMETA(DisplayName = "Light Attack (Phase 1)"),
    DashAttack = 2 UMETA(DisplayName = "Dash Attack (Phase 2)"),
    HeavyAttack = 3 UMETA(DisplayName = "Heavy Attack (Phase 3)")
};

USTRUCT(BlueprintType)
struct FStatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FGameplayTag StatTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsMultiplier = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Value = 0.0f;
};

UCLASS()
class PROJBLANK_API ACPP_BaseCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ACPP_BaseCharacter(const FObjectInitializer& ObjectInitializer);
    virtual void OnJumped_Implementation() override;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
        class AController* EventInstigator, AActor* DamageCauser) override;
    UFUNCTION(BlueprintCallable, Category = "Stats")
        void ApplyStatModifier(FStatModifier Modifier);
    //delegate object
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnNotificationReceived OnNotificationReceived;

    void ShowNotification(FText Text, FLinearColor Color = FLinearColor::White);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
        int32 GrantAbility(FGameplayTag AbilityTag, int32 MaxLevel);

    UFUNCTION(BlueprintCallable, Category = "Abilities")
        bool HasAbility(FGameplayTag AbilityTag) const;
    UFUNCTION(BlueprintCallable, Category = "Abilities")
        int32 GetAbilityLevel(FGameplayTag AbilityTag) const;

    void CastFireball();

    virtual bool CanDealDamageTo(AActor* TargetActor) const;

    //coins
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnCoinsUpdated OnCoinsUpdated;

    UFUNCTION(BlueprintCallable, Category = "Economy")
        void AddCoins(int32 Amount);
    int32 GetCoinCount() const { return CoinCount; }

    //xp
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

    //GETTERS 
    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        float GetCurrentMaxHealth() const { return CurrentMaxHealth; }

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        float GetCurrentBaseDamage() const { return CurrentBaseDamage; }

    const TMap<FGameplayTag, int32>& GetAbilityLevels() const { return AbilityLevels; }

    //SETTERS
    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        void SetStatsFromSave(float SavedHealth, float SavedMaxHealth, float SavedBaseDamage, int32 SavedLevel, float SavedXP, int32 SavedCoins);

    void SetAbilityLevels(const TMap<FGameplayTag, int32>& LoadedAbilities);

    void SetLocationFromSave(FVector SavedLocation);

    void SetCurrentHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "State")
        bool IsDead() const { return bIsDead; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
        UCharacterStats* CharacterStats;

    //flipbook
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
        UPaperFlipbook* IdleAnimationFlipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
        UPaperFlipbook* WalkAnimationFlipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Jumping")
        UPaperFlipbook* JumpAnimationFlipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Jumping")
        UPaperFlipbook* FallingAnimationFlipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Jumping")
        UPaperFlipbook* LandingAnimationFlipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Actions")
        UPaperFlipbook* DodgeAnimationFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Death")
        UPaperFlipbook* DeathAnimationFlipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Death")
        UPaperFlipbook* DeadStaticFlipbook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* Attack1Flipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* Attack2Flipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* Attack3Flipbook;
    // BlueprintReadWrite states
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
        bool bIsInAir = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsDodging = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsJumping = false;

    //attacks
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State | Attack")
        bool bIsAttacking = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State | Attack")
        int32 ComboCounter = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State | Attack")
        bool bCanCastSpell = true;

    FTimerHandle SpellCooldownTimer;
    void ResetSpellCooldown();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat | Spells")
        float FireballCooldown = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Dynamic")
        float CurrentBaseDamage = 0.f;;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Dynamic")
        float CurrentDamageMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsDead = false;

    //health
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
        float CurrentHealth;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
        float CurrentMaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsInvulnerable = false;

    //unlocked abilities
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
        TMap<FGameplayTag, int32> AbilityLevels;

    // projectile
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat | Spells")
        TArray<TSubclassOf<class ACPP_Projectile>> FireballLevels;

    //functions
	virtual void SetupPlayerInputComponent(class UInputComponent*
		PlayerInputComponent) override;

    UFUNCTION(BlueprintNativeEvent, Category = "Health")
        void OnDeath();

    void SpawnParticle(UNiagaraSystem* Effect, FVector Location, 
        FRotator Rotation = FRotator::ZeroRotator);

    //currency economics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
        int32 CoinCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
        float CoinMultiplier = 1.0f;

    //progression
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        int32 CharacterLevel = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        float CurrentXP = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
        float XPToNextLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
        float LevelUpMultiplier = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat | UI")
        TSubclassOf<class ACPP_DamageTextActor> DamageTextClass;

    void LevelUp();

	//movement
	void MoveRight(float Value);
	void MoveForward(float Value);
	void Dodge();
    void StopDodge();
    void StopJump();

    void Attack();
    void AttackEnd();
    void ResetCombo();

    void SwitchToDeadStatic();
    void DestroyCharacter();

    void InteractWithObject();

    void PerformAttackTrace(float Range, FVector BoxSize, float DamageAmount);

    virtual void BeginPlay() override;

    FTimerHandle ComboResetTimer;
};
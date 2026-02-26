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
#include "CPP_AttributeComponent.h"
#include "CPP_Action.h"   
#include "CPP_ActionComponent.h"
#include "CPP_BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, MaxHealth);

class UPaperFlipbook;

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
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
        class AController* EventInstigator, AActor* DamageCauser) override;
    UFUNCTION(BlueprintCallable, Category = "Stats")
        virtual void ApplyStatModifier(FStatModifier Modifier);
    //delegate object
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnHealthChangedSignature OnHealthChanged;

    virtual bool CanDealDamageTo(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        float GetCurrentHealth() const;

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        float GetCurrentMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
        float GetCurrentBaseDamage() const { return CurrentBaseDamage; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
        class UCharacterStats* GetCharacterStats() const { return CharacterStats; }

    //SETTERS
    void SetAbilityLevels(const TMap<FGameplayTag, int32>& LoadedAbilities);

    void SetLocationFromSave(FVector SavedLocation);

    void SetCurrentHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "State")
        bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "State")
        bool GetIsAttacking() const;

    //attack trace box 
    void PerformAttackTrace(float Range, FVector BoxSize, float DamageAmount);

protected:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
        UCharacterStats* CharacterStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UCPP_AttributeComponent* AttributeComp;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
        float BaseSpriteScale = 1.0f;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UCPP_ActionComponent* ActionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Dynamic")
        float CurrentBaseDamage = 0.f;;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Dynamic")
        float CurrentDamageMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsDead = false;

    //health
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsInvulnerable = false;

    //functions

    UFUNCTION(BlueprintNativeEvent, Category = "Health")
        void OnDeath();

    UFUNCTION()
        void OnDeathStarted(AActor* Killer);

    UFUNCTION()
        void OnHealthChangedCallback(AActor* InstigatorActor, UCPP_AttributeComponent* OwningComp, float NewHealth, float Delta);
    
    void SpawnParticle(UNiagaraSystem* Effect, FVector Location, 
        FRotator Rotation = FRotator::ZeroRotator);


    UPROPERTY(EditDefaultsOnly, Category = "Combat | UI")
        TSubclassOf<class ACPP_DamageTextActor> DamageTextClass;

	//movement

    void SwitchToDeadStatic();
    void DestroyCharacter();

    virtual void BeginPlay() override;

    FTimerHandle ComboResetTimer;

    virtual void Tick(float DeltaTime) override;
};
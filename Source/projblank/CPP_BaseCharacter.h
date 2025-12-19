// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterStats.h"
#include "InteractableInterface.h"
#include "CPP_BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, MaxHealth);

class UPaperFlipbook;

UENUM(BlueprintType)
enum class EAttackPhase : uint8
{
    None = 0 UMETA(DisplayName = "No Attack"),
    LightAttack = 1 UMETA(DisplayName = "Light Attack (Phase 1)"),
    DashAttack = 2 UMETA(DisplayName = "Dash Attack (Phase 2)"),
    HeavyAttack = 3 UMETA(DisplayName = "Heavy Attack (Phase 3)")
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

    //delegate object
    UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnHealthChangedSignature OnHealthChanged;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsDead = false;

    //health
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
        float CurrentHealth;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
        float MaxHealth;
    //functions
	virtual void SetupPlayerInputComponent(class UInputComponent*
		PlayerInputComponent) override;

    UFUNCTION(BlueprintNativeEvent, Category = "Health")
        void OnDeath();

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
    virtual bool CanDealDamageTo(AActor* TargetActor) const;

    FTimerHandle ComboResetTimer;
};
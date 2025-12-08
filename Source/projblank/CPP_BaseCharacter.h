// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterStats.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* Attack1Flipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* Attack2Flipbook;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations | Attack")
        UPaperFlipbook* Attack3Flipbook;
    // BlueprintReadWrite, чтобы мы могли менять его в Блюпринтах
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

    //health
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
        float MaxHealth = 100.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
        float CurrentHealth;

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

    virtual void BeginPlay() override;

    FTimerHandle ComboResetTimer;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CPP_BaseCharacter.generated.h"

class UPaperFlipbook;

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_BaseCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ACPP_BaseCharacter(const FObjectInitializer& ObjectInitializer);
    virtual void OnJumped_Implementation() override;
protected:
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
    // BlueprintReadWrite, чтобы мы могли менять его в Блюпринтах
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
        bool bIsInAir = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsDodging = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        bool bIsJumping = false;

	virtual void SetupPlayerInputComponent(class UInputComponent*
		PlayerInputComponent) override;

	//movement
	void MoveRight(float Value);
	void MoveForward(float Value);
	void Dodge();
    void StopDodge();
    void StopJump();
};
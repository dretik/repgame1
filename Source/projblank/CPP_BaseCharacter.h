// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CPP_BaseCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_BaseCharacter : public APaperCharacter
{
	GENERATED_BODY()

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent*
		PlayerInputComponent) override;

	void MoveRight(float Value);
	void MoveForward(float Value);

};
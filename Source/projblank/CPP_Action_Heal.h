// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_Heal.generated.h"

class UPaperFlipbook;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EHealType : uint8
{
	Flat        UMETA(DisplayName = "Fixed Amount"),
	Percentage  UMETA(DisplayName = "Percentage of Max HP")
};

UCLASS()
class PROJBLANK_API UCPP_Action_Heal : public UCPP_Action
{
	GENERATED_BODY()
	
public:
	UCPP_Action_Heal();

	virtual void StartAction_Implementation(AActor* Instigator) override;

	// ѕереопредел€ем проверку старта, чтобы босс не лечилс€, если он полон здоровь€
	virtual bool CanStart_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Heal | Config")
		EHealType HealType = EHealType::Flat;

	UPROPERTY(EditDefaultsOnly, Category = "Heal | Config")
		float HealAmount = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Heal | Config")
		bool bApplyLevelScaling = true;

	// «адержка перед применением лечени€ (чтобы совпало с анимацией)
	UPROPERTY(EditDefaultsOnly, Category = "Heal | Config")
		float HealDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		UPaperFlipbook* HealAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		UNiagaraSystem* HealVFX;

	FTimerHandle TimerHandle_HealDelay;

	UFUNCTION()
		void ExecuteHeal(AActor* Instigator);
};

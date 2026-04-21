// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_AttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChanged, AActor*, InstigatorActor, class UCPP_AttributeComponent*, OwningComp, float, NewValue, float, Delta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KillerActor);


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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJBLANK_API UCPP_AttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPP_AttributeComponent();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
		float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
		float DamageMultiplier=1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
		float BaseSpeed = 450.0f;

	UPROPERTY()
		TMap<UObject*, float> SpeedMultipliers;

	void UpdateActualMovementSpeed();

	bool bIsDead = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
		bool ApplyHealthChange(AActor* InstigatorActor, float Delta);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		float GetBaseSpeed() const { return BaseSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		void InitializeStats(float NewMaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		void SetDamageMultiplier(float NewMultiplier) { DamageMultiplier = NewMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
		float GetDamageMultiplier() const { return DamageMultiplier; }

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
		FOnAttributeChanged OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Attributes|Speed")
		void SetBaseSpeed(float NewBase);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Speed")
		void AddSpeedMultiplier(UObject* Source, float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Attributes|Speed")
		void RemoveSpeedMultiplier(UObject* Source);

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
		FOnDeath OnDeath;
};
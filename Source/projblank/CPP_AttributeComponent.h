// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_AttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChanged, AActor*, InstigatorActor, class UCPP_AttributeComponent*, OwningComp, float, NewValue, float, Delta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KillerActor);

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
		void InitializeStats(float NewMaxHealth);

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
		FOnAttributeChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
		FOnDeath OnDeath;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_AttributeComponent.generated.h"

// Делегат для уведомления UI и других систем об изменении здоровья
// Instigator - кто нанес урон (полезно для лога боя)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChanged, AActor*, InstigatorActor, class UCPP_AttributeComponent*, OwningComp, float, NewValue, float, Delta);

// Делегат смерти (чтобы не проверять каждый кадр <= 0)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KillerActor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJBLANK_API UCPP_AttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPP_AttributeComponent();

protected:
	// Скрываем изменение переменных напрямую, только через функции
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
		float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (AllowPrivateAccess = "true"))
		float MaxHealth;

	// Флаг, чтобы не триггерить смерть дважды
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

	// Позволяет инициализировать статы из DataAsset при старте
	UFUNCTION(BlueprintCallable, Category = "Attributes")
		void InitializeStats(float NewMaxHealth);

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
		FOnAttributeChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
		FOnDeath OnDeath;
};
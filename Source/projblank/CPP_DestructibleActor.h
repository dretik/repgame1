// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_DestructibleActor.generated.h"

UCLASS()
class PROJBLANK_API ACPP_DestructibleActor : public AActor
{
    GENERATED_BODY()

public:
    ACPP_DestructibleActor();

protected:
    // Модуль здоровья
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UCPP_AttributeComponent* AttributeComp;

    // Визуальная часть
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UStaticMeshComponent* MeshComp;

    // Ссылка на лут, который выпадет
    UPROPERTY(EditAnywhere, Category = "Loot")
        TSubclassOf<class AActor> LootClass;

    // Коллбэк для смерти
    UFUNCTION()
        void OnDestroyed(AActor* Killer);

    virtual void BeginPlay() override;

public:
    // Стандартная функция получения урона в UE4 работает для ВСЕХ Actor-ов
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
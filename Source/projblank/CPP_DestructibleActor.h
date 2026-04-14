// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_DamageableInterface.h"
#include "GameFramework/Actor.h"
#include "CPP_DestructibleActor.generated.h"

UCLASS()
class PROJBLANK_API ACPP_DestructibleActor : public AActor, public ICPP_DamageableInterface
{
    GENERATED_BODY()

public:
    ACPP_DestructibleActor();

    //interface realisation
    virtual class UCPP_AttributeComponent* GetAttributeComponent_Implementation() const override { return AttributeComp; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UCPP_AttributeComponent* AttributeComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UCPP_VisualComponent* VisualComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        class UStaticMeshComponent* MeshComp;

    //loot
    UPROPERTY(EditAnywhere, Category = "Loot")
        TSubclassOf<class AActor> LootClass;

    UPROPERTY(EditAnywhere, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
        float DropChance = 0.5f;

    //destruction
    UPROPERTY(EditAnywhere, Category = "Visuals")
        class UNiagaraSystem* BreakEffect;

    UFUNCTION()
        void OnDestroyed(AActor* Killer);

    virtual void BeginPlay() override;

public:
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
        AController* EventInstigator, AActor* DamageCauser) override;
};
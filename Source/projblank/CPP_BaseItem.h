// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseItem.generated.h"

class UPaperSpriteComponent;
class USphereComponent;

UCLASS()
class PROJBLANK_API ACPP_BaseItem : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    ACPP_BaseItem();

protected:
    // collision mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USphereComponent* SphereComp;

    // sprite
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UPaperSpriteComponent* ItemSprite;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
        bool bAutoPickup = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data | Stats")
        TArray<FStatModifier> StatModifiers;

    virtual void BeginPlay() override;

    UFUNCTION()
        void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
            UPrimitiveComponent* OtherComp, 
            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
    virtual void Interact_Implementation(AActor* Interactor) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
        FName ItemName;
};
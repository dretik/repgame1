// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseItem.generated.h"

class UPaperSpriteComponent;
class USphereComponent;
class UWidgetComponent;

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
    void EnablePhysicsCollision();

    // sprite
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UPaperSpriteComponent* ItemSprite;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USphereComponent* InteractionSphere;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data | UI")
        UTexture2D* ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
        bool bAutoPickup = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
        bool bIsInventoryItem = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data | Stats")
        TArray<FStatModifier> StatModifiers;

    virtual void BeginPlay() override;

    UFUNCTION()
        void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
            UPrimitiveComponent* OtherComp, 
            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data | Ability")
        FGameplayTag AbilityToUnlock;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data | Ability")
        int32 MaxAbilityLevel = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UWidgetComponent* LabelWidget;

public:
    virtual void Interact_Implementation(AActor* Interactor) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
        FName ItemName;

    //ui getters
    UTexture2D* GetIcon() const { return ItemIcon; }
    bool IsInventoryItem() const { return bIsInventoryItem; }

    FGameplayTag GetAbilityTag() const { return AbilityToUnlock; }
    int32 GetMaxLevel() const { return MaxAbilityLevel; }
};
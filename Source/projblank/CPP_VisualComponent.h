// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPP_VisualComponent.generated.h"

class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJBLANK_API UCPP_VisualComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCPP_VisualComponent();

    //hit reaction
    UFUNCTION(BlueprintCallable, Category = "Visuals")
        void PlayHitFlash(float Duration = 0.15f, FLinearColor FlashColor = FLinearColor::White);
    
    void HandleDamageReceived(float DamageAmount, TSubclassOf<AActor> TextClass, UNiagaraSystem* HitFX);

    void HandleDeath(UNiagaraSystem* DeathFX);

    void UpdateSpriteFacing(FVector Velocity, float BaseScale);
protected:
    UPROPERTY()
        class UPaperFlipbookComponent* MySprite;

    UPROPERTY()
        class UMaterialInstanceDynamic* DynamicMaterial;

    FTimerHandle TimerHandle_Flash;

    void OnFlashTimerExpired();

    virtual void BeginPlay() override;
};
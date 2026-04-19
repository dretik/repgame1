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
    
    //constant color overlay for statuses
    UFUNCTION(BlueprintCallable, Category = "Visuals")
        void SetStatusOverlay(FLinearColor OverlayColor, float Intensity,float EmissiveBoost);
    UFUNCTION(BlueprintCallable, Category = "Visuals")
        void ClearStatusOverlay();
    // getter for mesh to attach effects
    UFUNCTION(BlueprintCallable, Category = "Visuals")
        UMeshComponent* GetMesh() const { return MeshComp; }

    void HandleDamageReceived(float DamageAmount, TSubclassOf<AActor> TextClass, UNiagaraSystem* HitFX);

    void HandleDeath(UNiagaraSystem* DeathFX);

    void UpdateSpriteFacing(FVector Velocity, float BaseScale);

    UFUNCTION(BlueprintCallable, Category = "Visuals")
        void LockFlipping(float Duration);

    void FaceLocation(FVector TargetLocation, float BaseScale);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
        bool bInvertVisualFlipping = false;
protected:
    UPROPERTY()
        class UMeshComponent* MeshComp;

    UPROPERTY()
        class UMaterialInstanceDynamic* DynamicMaterial;

    FTimerHandle TimerHandle_Flash;

    void OnFlashTimerExpired();

    virtual void BeginPlay() override;

    void ResetMaterialParameters();

    bool bFlippingLocked = false;
    FTimerHandle TimerHandle_FlipLock;

    void UnlockFlipping() { bFlippingLocked = false; }
    void ApplyFlipping(float DesiredWorldDirY, float BaseScale);
};
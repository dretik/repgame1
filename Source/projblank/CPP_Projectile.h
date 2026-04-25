// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_Projectile.generated.h"

class UPaperFlipbookComponent;
class USphereComponent;
class UProjectileMovementComponent;
class UPaperFlipbook;
class UNiagaraSystem;
class UCPP_Action_Effect;

UCLASS()
class PROJBLANK_API ACPP_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Projectile();

    void SetDamage(float NewDamage) { Damage = NewDamage; }

    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SetPersistentEffects(TArray<TSubclassOf<UCPP_Action_Effect>> NewEffects) 
    { StatusEffects = NewEffects; }

    UFUNCTION(BlueprintCallable, Category = "Projectile")
        void SetProjectileStats(float NewSpeed, float NewRadius);

protected:
    //components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USphereComponent* CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UPaperFlipbookComponent* ProjectileSprite;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UProjectileMovementComponent* MovementComp;

    // sprite
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
        UPaperFlipbook* SpawnAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
        UPaperFlipbook* FlyAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
        UPaperFlipbook* HitAnim;

    //combat
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
        float Damage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
        float ExplosionRadius = 80.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
        float ImpulseStrength = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
        UNiagaraSystem* ExplosionEffect;

    UPROPERTY(EditAnywhere, Category = "Combat")
        TArray<TSubclassOf<UCPP_Action_Effect>> StatusEffects;

    virtual void BeginPlay() override;

    void SwitchToFlyLoop();

    UFUNCTION()
        void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    UFUNCTION()
        void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
        void Explode(AActor* Target);

    void DestroyProjectile();

    bool bHasExploded = false;
};

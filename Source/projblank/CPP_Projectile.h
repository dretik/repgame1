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

UCLASS()
class PROJBLANK_API ACPP_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Projectile();

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
        UNiagaraSystem* ExplosionEffect;

    virtual void BeginPlay() override;

    void SwitchToFlyLoop();

    UFUNCTION()
        void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void DestroyProjectile();

    bool bHasExploded = false;

public:	

};

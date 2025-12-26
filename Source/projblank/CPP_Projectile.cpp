// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_Projectile.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "DrawDebugHelpers.h"
#include "CPP_BaseCharacter.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h" 

ACPP_Projectile::ACPP_Projectile()
{
    // collision
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->SetSphereRadius(10.0f);
    CollisionComp->SetCollisionProfileName("Projectile"); // or blockalldynamics

    //sprite
    ProjectileSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("ProjectileSprite"));
    ProjectileSprite->SetupAttachment(RootComponent);
    ProjectileSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //movement
    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
    MovementComp->UpdatedComponent = CollisionComp;
    MovementComp->InitialSpeed = 600.0f;
    MovementComp->MaxSpeed = 600.0f;
    MovementComp->bShouldBounce = false;
    MovementComp->ProjectileGravityScale = 0.1f;
}

void ACPP_Projectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionComp->OnComponentHit.AddDynamic(this, &ACPP_Projectile::OnHit);

    if (SpawnAnim)
    {
        ProjectileSprite->SetFlipbook(SpawnAnim);
        ProjectileSprite->SetLooping(false); 

        float Duration = SpawnAnim->GetTotalDuration();
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, this, &ACPP_Projectile::SwitchToFlyLoop, Duration, false);
    }
    else
    {
        SwitchToFlyLoop();
    }
}

void ACPP_Projectile::SwitchToFlyLoop()
{
    if (bHasExploded) return;

    if (FlyAnim)
    {
        ProjectileSprite->SetFlipbook(FlyAnim);
        ProjectileSprite->SetLooping(true);
        ProjectileSprite->Play();
    }
}

void ACPP_Projectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bHasExploded) return;

    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetOwner()))
    {
        bHasExploded = true;

        MovementComp->StopMovementImmediately();
        CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        float DestroyDelay = 0.1f;
        if (HitAnim)
        {
            ProjectileSprite->SetLooping(false);
            ProjectileSprite->SetFlipbook(HitAnim);
            ProjectileSprite->PlayFromStart();
            DestroyDelay = HitAnim->GetTotalDuration();
        }

        TArray<AActor*> OverlappedActors;
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.Add(this);
        if (GetOwner()) ActorsToIgnore.Add(GetOwner());

        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

        if (ExplosionEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
        }

        bool bResult = UKismetSystemLibrary::SphereOverlapActors(
            GetWorld(),
            GetActorLocation(),
            ExplosionRadius,
            ObjectTypes,
            AActor::StaticClass(),
            ActorsToIgnore,
            OverlappedActors
        );

        if (GEngine) DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Orange, false, 2.0f);
        
        if (bResult)
        {
            ACPP_BaseCharacter* OwnerChar = Cast<ACPP_BaseCharacter>(GetOwner());

            for (AActor* Target : OverlappedActors)
            {
                bool bCanDamage = true;
                if (OwnerChar)
                {
                    bCanDamage = OwnerChar->CanDealDamageTo(Target);
                }

                if (bCanDamage)
                {
                    UGameplayStatics::ApplyDamage(
                        Target,
                        Damage,
                        GetInstigatorController(),
                        this, // damage dealt
                        UDamageType::StaticClass()
                    );
                }
            }
        }

        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, this, &ACPP_Projectile::DestroyProjectile, DestroyDelay, false);
    }
}

void ACPP_Projectile::DestroyProjectile()
{
    Destroy();
}
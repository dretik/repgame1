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
#include "CPP_Action_Effect.h"
#include "CPP_CombatStatics.h"
#include "CPP_VisualComponent.h"

ACPP_Projectile::ACPP_Projectile()
{
    // collision
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->SetSphereRadius(10.0f);
    CollisionComp->SetCollisionProfileName("Projectile"); // or blockalldynamics

    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    CollisionComp->bTraceComplexOnMove = true;
    CollisionComp->BodyInstance.bUseCCD = true;

    VisualComp = CreateDefaultSubobject<UCPP_VisualComponent>(TEXT("VisualComp"));
    PrimaryActorTick.bCanEverTick = true;

    //sprite
    ProjectileSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("ProjectileSprite"));
    ProjectileSprite->SetupAttachment(RootComponent);
    ProjectileSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileSprite->SetUsingAbsoluteRotation(true);
    ProjectileSprite->SetWorldRotation(FRotator(0, 90, 0));

    //movement
    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
    MovementComp->UpdatedComponent = CollisionComp;
    MovementComp->InitialSpeed = 600.0f;
    MovementComp->MaxSpeed = 600.0f;
    MovementComp->bShouldBounce = false;
    MovementComp->ProjectileGravityScale = 0.1f;
}

void ACPP_Projectile::SetProjectileStats(float NewSpeed, float NewRadius)
{
    if (MovementComp) {
        MovementComp->InitialSpeed = NewSpeed;
        MovementComp->MaxSpeed = NewSpeed;
        MovementComp->Velocity = GetActorForwardVector() * NewSpeed;
    }
    ExplosionRadius = NewRadius;
}

void ACPP_Projectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionComp->OnComponentHit.AddDynamic(this, &ACPP_Projectile::OnHit);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Projectile::OnOverlap);

    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
        //if (UPrimitiveComponent* OwnerRoot = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent()))
        //{
        //    OwnerRoot->IgnoreActorWhenMoving(this, true);
        //}
    }

    ProjectileSprite->SetWorldRotation(FRotator(0, 90.0f, 0));
    BaseSpriteScale = FMath::Abs(ProjectileSprite->GetRelativeScale3D().X);

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
    Explode(OtherActor);
}

void ACPP_Projectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bHasExploded || !OtherActor || OtherActor == GetOwner()) return;

    if (OtherComp && OtherComp->GetCollisionProfileName() == FName("Trigger"))
    {
        return;
    }

    bool bCanDamage = false;
    AActor* MyOwner = GetOwner();

    if (MyOwner && MyOwner->GetClass()->ImplementsInterface(UCPP_CombatInterface::StaticClass()))
    {
        bCanDamage = ICPP_CombatInterface::Execute_CanDealDamageTo(MyOwner, OtherActor);
    }

    if (bCanDamage)
    {
        Explode(OtherActor);
    }
}

void ACPP_Projectile::Explode(AActor* Target)
{
    if (bHasExploded) return;
    bHasExploded = true;

    MovementComp->StopMovementImmediately();
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (HitAnim) {
        ProjectileSprite->SetLooping(false);
        ProjectileSprite->SetFlipbook(HitAnim);
        ProjectileSprite->PlayFromStart();
    }
    if (ExplosionEffect) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    }

    UCPP_CombatStatics::ExecuteAreaDamage(
        this, GetOwner(), GetActorLocation(), ExplosionRadius, Damage,
        StatusEffects, ImpulseStrength, true
    );
    FTimerHandle TimerHandle_Destroy;
    float DestroyDelay = HitAnim ? HitAnim->GetTotalDuration() : 0.1f;
    GetWorldTimerManager().SetTimer(TimerHandle_Destroy, this, &ACPP_Projectile::DestroyProjectile, DestroyDelay, false);
}

void ACPP_Projectile::DestroyProjectile()
{
    Destroy();
}

void ACPP_Projectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (VisualComp) {
        VisualComp->UpdateSpriteFacing(GetVelocity(), BaseSpriteScale);
    }
}
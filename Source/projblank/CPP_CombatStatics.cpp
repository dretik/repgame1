// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_CombatStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "CPP_AttributeComponent.h"
#include "CPP_CombatInterface.h"
#include "CPP_ActionComponent.h"
#include "CPP_Action_Effect.h"
#include "CPP_Projectile.h"
#include "CPP_VisualComponent.h"
#include "DrawDebugHelpers.h"

bool UCPP_CombatStatics::ExecuteBoxTraceAttack(AActor* DamageCauser, AActor* Instigator, 
    FVector Origin, FVector AttackDirection, 
    float Range, FVector BoxSize, float BaseDamage, 
    const TArray<TSubclassOf<UCPP_Action_Effect>>& EffectsToApply,
    float ImpulseStrength, bool bDrawDebug)
{
    if (!DamageCauser) return false;

    UWorld* World = DamageCauser->GetWorld();
    if (!World) return false;

    //getting dmg multiplier (if is in Instigator)
    float FinalDamage = BaseDamage;
    if (Instigator)
    {
        UCPP_AttributeComponent* AttrComp = Instigator->FindComponentByClass<UCPP_AttributeComponent>();
        if (AttrComp)
        {
            FinalDamage *= AttrComp->GetDamageMultiplier();
        }
    }

    //trace calc
    FVector Start = Origin;
    FVector End = Start + (AttackDirection.GetSafeNormal() * Range);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(DamageCauser);
    if (Instigator && Instigator != DamageCauser)
    {
        ActorsToIgnore.Add(Instigator);
    }

    TArray<FHitResult> HitResults;
    EDrawDebugTrace::Type DebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

    bool bHit = UKismetSystemLibrary::BoxTraceMulti(
        World, Start, End, BoxSize, FRotator::ZeroRotator,
        UEngineTypes::ConvertToTraceType(ECC_WorldDynamic), false,
        ActorsToIgnore, DebugType, HitResults, true
    );

    if (!bHit) return false;

    //target filter and damage applience
    TSet<AActor*> DamagedActors;
    ICPP_CombatInterface* CombatInterfacer = Cast<ICPP_CombatInterface>(Instigator ? Instigator : DamageCauser);

    for (const FHitResult& Result : HitResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!HitActor || DamagedActors.Contains(HitActor)) continue;

        // delegating interface checking if no interface present - damage can be dealt
        bool bCanDamage = true;
        UObject* InterfaceObj = Instigator ? (UObject*)Instigator : (UObject*)DamageCauser;

        if (InterfaceObj)
        {
            bCanDamage = ICPP_CombatInterface::Execute_CanDealDamageTo(InterfaceObj, HitActor);
        }

        if (bCanDamage)
        {
            DamagedActors.Add(HitActor);
            UGameplayStatics::ApplyDamage(
                HitActor, FinalDamage,
                Instigator ? Instigator->GetInstigatorController() : nullptr,
                DamageCauser, UDamageType::StaticClass()
            );
            //status effects
            if (EffectsToApply.Num() > 0)
            {
                UCPP_ActionComponent* ActionComp = HitActor->FindComponentByClass<UCPP_ActionComponent>();
                if (ActionComp)
                {
                    for (TSubclassOf<UCPP_Action_Effect> EffectClass : EffectsToApply)
                    {
                        ActionComp->ApplyStatusEffect(EffectClass, Instigator);
                    }
                }
            }

            //impulse
            if (ImpulseStrength > 0.0f)
            {
                FVector ImpulseDir = AttackDirection.GetSafeNormal();
                ImpulseDir.Z = 0.4f;

                if (ACharacter* TargetChar = Cast<ACharacter>(HitActor))
                {
                    TargetChar->LaunchCharacter(ImpulseDir * ImpulseStrength, true, true);
                }
                else if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
                {
                    if (RootComp->IsSimulatingPhysics())
                    {
                        RootComp->AddImpulse(ImpulseDir * ImpulseStrength, NAME_None, true);
                    }
                }
            }
        }
    }

    return DamagedActors.Num() > 0;
}

bool UCPP_CombatStatics::ExecuteAreaDamage(AActor* DamageCauser, AActor* Instigator, 
    FVector Origin, float Radius, float BaseDamage, 
    const TArray<TSubclassOf<UCPP_Action_Effect>>& EffectsToApply,
    float ImpulseStrength, bool bDrawDebug)
{
    if (!DamageCauser) return false;

    UWorld* World = DamageCauser->GetWorld();
    if (!World) return false;

    float FinalDamage = BaseDamage;
    if (Instigator)
    {
        UCPP_AttributeComponent* AttrComp = Instigator->FindComponentByClass<UCPP_AttributeComponent>();
        if (AttrComp) FinalDamage *= AttrComp->GetDamageMultiplier();
    }

    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(DamageCauser);
    if (Instigator) ActorsToIgnore.Add(Instigator);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

    bool bResult = UKismetSystemLibrary::SphereOverlapActors(
        World, Origin, Radius, ObjectTypes, AActor::StaticClass(), ActorsToIgnore, OverlappedActors
    );

    if (bDrawDebug)
    {
        DrawDebugSphere(World, Origin, Radius, 12, FColor::Orange, false, 2.0f);
    }

    if (!bResult) return false;

    ICPP_CombatInterface* CombatInterfacer = Cast<ICPP_CombatInterface>(Instigator ? Instigator : DamageCauser);

    for (AActor* Target : OverlappedActors)
    {
        if (!Target) continue;
        bool bCanDamage = true;
        UObject* InterfaceObj = nullptr;
        if (Instigator && Instigator->GetClass()->ImplementsInterface(UCPP_CombatInterface::StaticClass()))
        {
            InterfaceObj = Instigator;
        }
        else if (DamageCauser && DamageCauser->GetClass()->ImplementsInterface(UCPP_CombatInterface::StaticClass()))
        {
            InterfaceObj = DamageCauser;
        }
        if (Instigator && Instigator->GetClass()->ImplementsInterface(UCPP_CombatInterface::StaticClass()))
        {
            bCanDamage = ICPP_CombatInterface::Execute_CanDealDamageTo(Instigator, Target);
        }
        else if (DamageCauser && DamageCauser->GetClass()->ImplementsInterface(UCPP_CombatInterface::StaticClass()))
        {
            bCanDamage = ICPP_CombatInterface::Execute_CanDealDamageTo(DamageCauser, Target);
        }
        if (bCanDamage)
        {
            UGameplayStatics::ApplyDamage(
                Target, FinalDamage,
                Instigator ? Instigator->GetInstigatorController() : nullptr,
                DamageCauser, UDamageType::StaticClass()
            );
            //status effects
            if (EffectsToApply.Num() > 0)
            {
                UCPP_ActionComponent* ActionComp = Target->FindComponentByClass<UCPP_ActionComponent>();
                if (ActionComp)
                {
                    for (TSubclassOf<UCPP_Action_Effect> EffectClass : EffectsToApply)
                    {
                        ActionComp->ApplyStatusEffect(EffectClass, Instigator);
                    }
                }
            }
            //impulse
            FVector ImpulseDir = Target->GetActorLocation() - Origin;
            ImpulseDir.Normalize();
            ImpulseDir.Z = 0.5f;

            ACharacter* Character = Cast<ACharacter>(Target);
            if (Character)
            {
                Character->LaunchCharacter(ImpulseDir * ImpulseStrength, true, true);
            }
            else
            {
                UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(Target->GetRootComponent());
                if (RootComp && RootComp->IsSimulatingPhysics())
                {
                    RootComp->AddImpulse(ImpulseDir * ImpulseStrength, NAME_None, true);
                }
            }
        }
    }

    return true;
}

bool UCPP_CombatStatics::ExecuteHealing(AActor* Instigator, AActor* Target, float HealAmount, bool bIsPercentage)
{
    if (!Target) return false;
    UCPP_AttributeComponent* AttrComp = Target->FindComponentByClass<UCPP_AttributeComponent>();
    if (!AttrComp) return false;

    float FinalHeal = HealAmount;
    if (bIsPercentage)
    {
        FinalHeal = AttrComp->GetMaxHealth() * (HealAmount / 100.0f);
    }

    return AttrComp->ApplyHealthChange(Instigator, FinalHeal);
}

ACPP_Projectile* UCPP_CombatStatics::SpawnProjectile(AActor* Instigator, TSubclassOf<ACPP_Projectile> ProjectileClass, float Damage, float Speed, float Radius, float Scale, const TArray<TSubclassOf<UCPP_Action_Effect>>& EffectsToApply)
{
    if (!Instigator || !ProjectileClass) return nullptr;

    UWorld* World = Instigator->GetWorld();
    if (!World) return nullptr;

    UCPP_VisualComponent* VisualComp = Instigator->FindComponentByClass<UCPP_VisualComponent>();
    FVector FacingDir = VisualComp ? VisualComp->GetVisualFacingDirection() : Instigator->GetActorForwardVector();

    float BaseOffset = 40.0f * Scale;
    FVector SpawnLocation = Instigator->GetActorLocation();
    SpawnLocation.Y += FacingDir.Y * BaseOffset;
    SpawnLocation.Z += 20.0f;

    FRotator SpawnRotation = FRotator(0, (FacingDir.Y > 0 ? 90.f : -90.f), 0);

    FActorSpawnParameters Params;
    Params.Instigator = Cast<APawn>(Instigator);
    Params.Owner = Instigator;

    ACPP_Projectile* Proj = World->SpawnActor<ACPP_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

    if (Proj)
    {
        Proj->SetDamage(Damage);
        Proj->SetActorScale3D(FVector(Scale));
        Proj->SetProjectileStats(Speed, Radius);
        Proj->SetPersistentEffects(EffectsToApply);
    }

    return Proj;
}
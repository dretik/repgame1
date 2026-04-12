// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_CombatStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CPP_AttributeComponent.h"
#include "CPP_CombatInterface.h"
#include "DrawDebugHelpers.h"

bool UCPP_CombatStatics::ExecuteBoxTraceAttack(AActor* DamageCauser, AActor* Instigator, FVector Origin, FVector AttackDirection, float Range, FVector BoxSize, float BaseDamage, bool bDrawDebug)
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
        if (CombatInterfacer)
        {
            bCanDamage = CombatInterfacer->CanDealDamageTo(HitActor);
        }

        if (bCanDamage)
        {
            DamagedActors.Add(HitActor);
            UGameplayStatics::ApplyDamage(
                HitActor, FinalDamage,
                Instigator ? Instigator->GetInstigatorController() : nullptr,
                DamageCauser, UDamageType::StaticClass()
            );
        }
    }

    return DamagedActors.Num() > 0;
}

bool UCPP_CombatStatics::ExecuteAreaDamage(AActor* DamageCauser, AActor* Instigator, FVector Origin, float Radius, float BaseDamage, bool bDrawDebug)
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
        bool bCanDamage = true;
        if (CombatInterfacer)
        {
            bCanDamage = CombatInterfacer->CanDealDamageTo(Target);
        }

        if (bCanDamage)
        {
            UGameplayStatics::ApplyDamage(
                Target, FinalDamage,
                Instigator ? Instigator->GetInstigatorController() : nullptr,
                DamageCauser, UDamageType::StaticClass()
            );
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
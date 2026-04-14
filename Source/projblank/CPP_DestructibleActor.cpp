// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_DestructibleActor.h"
#include "CPP_AttributeComponent.h"
#include "CPP_VisualComponent.h"
#include "CPP_VisualStatics.h"
#include "Components/StaticMeshComponent.h"

ACPP_DestructibleActor::ACPP_DestructibleActor()
{
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    AttributeComp = CreateDefaultSubobject<UCPP_AttributeComponent>(TEXT("AttributeComp"));
    VisualComp = CreateDefaultSubobject<UCPP_VisualComponent>(TEXT("VisualComp"));
}

void ACPP_DestructibleActor::BeginPlay()
{
    Super::BeginPlay();
    //subscribing loot drop on component's death
    if(AttributeComp)
    AttributeComp->OnDeath.AddDynamic(this, &ACPP_DestructibleActor::OnDestroyed);
}

float ACPP_DestructibleActor::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!AttributeComp || AttributeComp->GetHealth() <= 0) return 0.0f;

    // 1. Применяем урон
    AttributeComp->ApplyHealthChange(DamageCauser, -DamageAmount);

    // 2. Визуальная реакция (Мигание)
    if (VisualComp) {
        VisualComp->PlayHitFlash(0.1f, FLinearColor::White);
    }

    // 3. Текст урона
    UCPP_VisualStatics::SpawnDamageText(this, nullptr, DamageAmount, GetActorLocation());

    return DamageAmount;
}

void ACPP_DestructibleActor::OnDestroyed(AActor* Killer)
{
    if (BreakEffect) {
        UCPP_VisualStatics::SpawnNiagaraEffect(this, BreakEffect, GetActorLocation());
    }

    if (LootClass && FMath::FRand() <= DropChance) {
        GetWorld()->SpawnActor<AActor>(LootClass, GetActorLocation(), GetActorRotation());
    }

    Destroy();
}
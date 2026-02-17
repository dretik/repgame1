// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_DestructibleActor.h"
#include "CPP_AttributeComponent.h"
#include "Components/StaticMeshComponent.h"

ACPP_DestructibleActor::ACPP_DestructibleActor()
{
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    // Просто вставляем наш модуль здоровья!
    AttributeComp = CreateDefaultSubobject<UCPP_AttributeComponent>(TEXT("AttributeComp"));
}

void ACPP_DestructibleActor::BeginPlay()
{
    Super::BeginPlay();
    // Подписываем логику выпадения лута на смерть компонента
    AttributeComp->OnDeath.AddDynamic(this, &ACPP_DestructibleActor::OnDestroyed);
}

float ACPP_DestructibleActor::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Просто пересылаем урон в компонент
    if (AttributeComp)
    {
        AttributeComp->ApplyHealthChange(DamageCauser, -DamageAmount);
    }
    return DamageAmount;
}

void ACPP_DestructibleActor::OnDestroyed(AActor* Killer)
{
    // Логика выпадения лута
    if (LootClass)
    {
        GetWorld()->SpawnActor<AActor>(LootClass, GetActorLocation(), GetActorRotation());
    }
    Destroy(); // Удаляем бочку
}
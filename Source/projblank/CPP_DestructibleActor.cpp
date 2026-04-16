// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_DestructibleActor.h"
#include "CPP_AttributeComponent.h"
#include "CPP_VisualComponent.h"
#include "CPP_VisualStatics.h"
#include "CPP_LootStatics.h"
#include "Components/StaticMeshComponent.h"
#include "CPP_SaveGame.h"
#include "CPP_GameInstance.h"

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

    AttributeComp->ApplyHealthChange(DamageCauser, -DamageAmount);

    if (VisualComp) {
        VisualComp->PlayHitFlash(0.1f, FLinearColor::White);
    }
    //null textclass - nothing to print out
    UCPP_VisualStatics::SpawnDamageText(this, nullptr, DamageAmount, GetActorLocation());

    return DamageAmount;
}

void ACPP_DestructibleActor::OnDestroyed(AActor* Killer)
{
    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
    if (GI)
    {
        GI->DestroyedStaticActors.Add(GetName());
    }

    if (BreakEffect) {
        UCPP_VisualStatics::SpawnNiagaraEffect(this, BreakEffect, GetActorLocation());
    }

    UCPP_LootStatics::SpawnAllLoot(this, ObjectStats, GetActorLocation());

    Destroy();
}

void ACPP_DestructibleActor::OnSaveGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject) return;

    FEnemySaveData Data;
    Data.bIsDead = false;
    Data.Location = GetActorLocation();

    SaveObject->WorldEnemies.Add(GetName(), Data);
}

void ACPP_DestructibleActor::OnLoadGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject) return;

    FString MyID = GetName();
    if (SaveObject->WorldEnemies.Contains(MyID))
    {
        if (SaveObject->WorldEnemies[MyID].bIsDead)
        {
            Destroy(); //if dead in save destroy on load
        }
    }
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_VisualStatics.h"
#include "CPP_VisualTextInterface.h"
#include "NiagaraFunctionLibrary.h"

void UCPP_VisualStatics::SpawnNiagaraEffect(const UObject* WorldContextObject, UNiagaraSystem* System, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!System || !WorldContextObject->GetWorld()) return;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        WorldContextObject->GetWorld(),
        System,
        Location,
        Rotation,
        Scale,
        true, // Auto Destroy
        true  // Auto Activate
    );
}

void UCPP_VisualStatics::SpawnDamageText(const UObject* WorldContextObject, TSubclassOf<AActor> TextClass, float DamageAmount, FVector Location)
{
    if (!TextClass || !WorldContextObject->GetWorld()) return;

    FVector SpawnLoc = Location + FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), FMath::RandRange(50.f, 70.f));

    AActor* TextActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(TextClass, SpawnLoc, FRotator::ZeroRotator);
    
    if (TextActor && TextActor->GetClass()->ImplementsInterface(UCPP_VisualTextInterface::StaticClass()))
    {
        ICPP_VisualTextInterface::Execute_SetValue(TextActor, DamageAmount);
    }
}
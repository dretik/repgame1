// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_VisualStatics.h"
#include "CPP_VisualTextInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AbilityCardData.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"

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

UNiagaraComponent* UCPP_VisualStatics::SpawnNiagaraEffectAttached(USceneComponent* AttachToComponent, UNiagaraSystem* System, FName SocketName, FVector RelativeLocation)
{
    if (!System || !AttachToComponent) return nullptr;

    return UNiagaraFunctionLibrary::SpawnSystemAttached(
        System,
        AttachToComponent,
        SocketName,
        RelativeLocation,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true // Auto Destroy
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

FLinearColor UCPP_VisualStatics::GetColorByRarity(ECardRarity Rarity)
{
    switch (Rarity)
    {
    case ECardRarity::Common:    return FLinearColor::White;
    case ECardRarity::Rare:      return FLinearColor(0.1f, 0.5f, 1.0f); // blue
    case ECardRarity::Epic:      return FLinearColor(0.6f, 0.1f, 1.0f); // purple
    case ECardRarity::Legendary: return FLinearColor(1.0f, 0.4f, 0.0f); // orange
    default:                     return FLinearColor::White;
    }
}

FText UCPP_VisualStatics::GetKeybindForAction(FName ActionName)
{
    const UInputSettings* Settings = GetDefault<UInputSettings>();
    if (!Settings) return FText::GetEmpty();

    // FInputActionKeyMapping
    TArray<FInputActionKeyMapping> OutMappings;
    Settings->GetActionMappingByName(ActionName, OutMappings);

    if (OutMappings.Num() > 0)
    {
        // OutMappings[0].Key - object type FKey
        return OutMappings[0].Key.GetDisplayName();
    }

    return FText::FromString("None");
}
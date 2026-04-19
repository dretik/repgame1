// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "CPP_VisualStatics.generated.h"

UCLASS()
class PROJBLANK_API UCPP_VisualStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Visuals", meta = (WorldContext = "WorldContextObject"))
        static void SpawnNiagaraEffect(const UObject* WorldContextObject, UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.f));

    UFUNCTION(BlueprintCallable, Category = "Visuals", meta = (WorldContext = "WorldContextObject"))
        static class UNiagaraComponent* SpawnNiagaraEffectAttached(USceneComponent* AttachToComponent, UNiagaraSystem* System, FName SocketName = NAME_None, FVector RelativeLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Visuals", meta = (WorldContext = "WorldContextObject"))
        static void SpawnDamageText(const UObject* WorldContextObject, TSubclassOf<AActor> TextClass, float DamageAmount, FVector Location);
};

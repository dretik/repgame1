// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Action_AreaPulse.h"
#include "CPP_VisualStatics.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UCPP_Action_AreaPulse::UCPP_Action_AreaPulse() { PulseInterval = 0.5f; }

void UCPP_Action_AreaPulse::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	FVector Loc = GetActionLocation();

	if (PersistentVFX)
		SpawnedVFX = UCPP_VisualStatics::SpawnNiagaraEffectAttached(Instigator->GetRootComponent(), PersistentVFX);

	if (SpawnedVFX) SpawnedVFX->SetWorldLocation(Loc);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Pulse, FTimerDelegate::CreateUObject(this, &UCPP_Action_AreaPulse::ExecutePulse, Instigator), PulseInterval, true);
}

void UCPP_Action_AreaPulse::ExecutePulse(AActor* Instigator)
{
	TArray<AActor*> OverlappedActors;
	TArray<AActor*> Ignore;
	Ignore.Add(Instigator);


	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActionLocation(), Radius, {}, AActor::StaticClass(), Ignore, OverlappedActors);

	OnPulse(OverlappedActors, Instigator);
}

void UCPP_Action_AreaPulse::StopAction_Implementation(AActor* Instigator)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Pulse);
	if (SpawnedVFX) { SpawnedVFX->Deactivate(); SpawnedVFX->DestroyComponent(); }
	Super::StopAction_Implementation(Instigator);
}
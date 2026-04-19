// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_Action_Effect.h"
#include "CPP_ActionComponent.h"
#include "CPP_AttributeComponent.h"
#include "CPP_VisualComponent.h"
#include "CPP_VisualStatics.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCPP_Action_Effect::UCPP_Action_Effect()
{
	bAutoStart = false;
	TickInterval = 1.0f;
	DamagePerTick = 0.0f;
	bCanStack = false;
	MaxStacks = 3;
	CurrentStacks = 1;
}

void UCPP_Action_Effect::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	CurrentStacks = 1;

	if (TickInterval > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_EffectTick, this, &UCPP_Action_Effect::ExecuteTick, TickInterval, true);
	}

	//visuals
	AActor* Owner = GetOwningComponent()->GetOwner();
	if (!Owner) return;

	UCPP_VisualComponent* VisComp = Owner->FindComponentByClass<UCPP_VisualComponent>();

	if (VisComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Applying Effect Color: %s"), *EffectColor.ToString());
		VisComp->SetStatusOverlay(EffectColor, StatusIntensity,EmissiveBoost);
	}

	if (PersistentFX)
	{
		//looking for mesh to attach to
		USceneComponent* AttachParent = VisComp ? Cast<USceneComponent>(VisComp->GetMesh()) : Owner->GetRootComponent();

		ActiveFXComp = UCPP_VisualStatics::SpawnNiagaraEffectAttached(AttachParent, PersistentFX);
	}
	//speed mod
	if (SpeedMultiplier != 1.0f)
	{
		UCPP_AttributeComponent* AttrComp = GetOwningComponent()->GetOwner()->FindComponentByClass<UCPP_AttributeComponent>();
		if (AttrComp)
		{
			AttrComp->AddSpeedMultiplier(this, SpeedMultiplier);
		}
	}
}

void UCPP_Action_Effect::StopAction_Implementation(AActor* Instigator)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_EffectTick);
	}

	if (SpeedMultiplier != 1.0f)
	{
		UCPP_AttributeComponent* AttrComp = GetOwningComponent()->GetOwner()->FindComponentByClass<UCPP_AttributeComponent>();
		if (AttrComp)
		{
			AttrComp->RemoveSpeedMultiplier(this);
		}
	}

	Super::StopAction_Implementation(Instigator);

	//visuals
	if (ActiveFXComp)
	{
		ActiveFXComp->Deactivate();
		ActiveFXComp->DestroyComponent();
	}

	//color reset
	if (AActor* Owner = GetOwningComponent()->GetOwner())
	{
		UCPP_VisualComponent* VisComp = Owner->FindComponentByClass<UCPP_VisualComponent>();
		if (VisComp) VisComp->ClearStatusOverlay();
	}
}

void UCPP_Action_Effect::RefreshAction_Implementation(AActor* Instigator)
{
	Super::RefreshAction_Implementation(Instigator);

	if (bCanStack && CurrentStacks < MaxStacks)
	{
		CurrentStacks++;
	}
}

void UCPP_Action_Effect::ExecuteTick()
{
	if (DamagePerTick == 0.0f) return;

	UCPP_ActionComponent* Comp = GetOwningComponent();
	if (!Comp || !Comp->GetOwner()) return;

	UCPP_AttributeComponent* AttrComp = Comp->GetOwner()->FindComponentByClass<UCPP_AttributeComponent>();
	if (AttrComp)
	{
		float Multiplier = 1.0f;
		if (RepData.Instigator)
		{
			UCPP_AttributeComponent* InstigatorAttr = RepData.Instigator->FindComponentByClass<UCPP_AttributeComponent>();
			if (InstigatorAttr) Multiplier = InstigatorAttr->GetDamageMultiplier();
		}

		float FinalDamage = DamagePerTick * CurrentStacks * Multiplier;

		AttrComp->ApplyHealthChange(RepData.Instigator, -FinalDamage);
	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Projectile_Card.h"
#include "CPP_ActionComponent.h"

void ACPP_Projectile_Card::Explode(AActor* Target)
{
	if (bHasExploded) return;

	if (ImpactAction && GetOwner())
	{
		UCPP_ActionComponent* ActionComp = GetOwner()->FindComponentByClass<UCPP_ActionComponent>();
		if (ActionComp)
		{
			// world coords could be
			ActionComp->ExecuteActionOnce(ImpactAction);
		}
	}

	Super::Explode(Target); //anim
}
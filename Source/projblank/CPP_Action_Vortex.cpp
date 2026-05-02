// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Action_Vortex.h"
#include "CPP_BaseCharacter.h"

void UCPP_Action_Vortex::OnPulse(const TArray<AActor*>& OverlappedActors, AActor* Instigator)
{
	for (AActor* Target : OverlappedActors)
	{
		ACharacter* Char = Cast<ACharacter>(Target);
		if (Char)
		{
			FVector Dir = GetActionLocation() - Char->GetActorLocation();
			Dir.Normalize();
			Char->LaunchCharacter(Dir * 1200.f, true, true);
		}
	}
}
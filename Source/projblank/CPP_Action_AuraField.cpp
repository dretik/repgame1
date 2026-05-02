#include "CPP_Action_AuraField.h"
#include "CPP_Action_Effect.h"
#include "CPP_ActionComponent.h"
#include "GameFramework/Character.h"
#include "CPP_CombatInterface.h"

UCPP_Action_AuraField::UCPP_Action_AuraField()
{
	PulseInterval = 0.2f;
}

void UCPP_Action_AuraField::OnPulse(const TArray<AActor*>& OverlappedActors, AActor* Instigator)
{
	FVector Center = GetActionLocation();

	for (AActor* Target : OverlappedActors)
	{
		if (!IsValidTarget(Target, Instigator)) continue;

		if (ForceType != EFieldForceType::None)
		{
			ACharacter* TargetChar = Cast<ACharacter>(Target);
			if (TargetChar)
			{
				FVector FinalForce = FVector::ZeroVector;

				if (ForceType == EFieldForceType::PullToCenter)
				{
					FinalForce = Center - TargetChar->GetActorLocation();
					FinalForce.Normalize();
					FinalForce *= ForceStrength;
				}
				else if (ForceType == EFieldForceType::PushFromCenter)
				{
					FinalForce = TargetChar->GetActorLocation() - Center;
					FinalForce.Normalize();
					FinalForce *= ForceStrength;
				}
				else if (ForceType == EFieldForceType::ConstantWind)
				{
					FinalForce = WindDirection.GetSafeNormal() * ForceStrength;
				}


				FinalForce.Z = FMath::Max(FinalForce.Z, 100.0f);

				TargetChar->LaunchCharacter(FinalForce, true, true);
			}
		}

		UCPP_ActionComponent* TargetAC = Target->FindComponentByClass<UCPP_ActionComponent>();
		if (TargetAC)
		{
			for (auto& EffectClass : EffectsToApply)
			{
				TargetAC->ApplyStatusEffect(EffectClass, Instigator);
			}
		}
	}
}
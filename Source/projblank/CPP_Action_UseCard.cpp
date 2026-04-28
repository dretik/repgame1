// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Action_UseCard.h"
#include "CPP_DeckComponent.h"
#include "CPP_ActionComponent.h"
#include "CPP_Projectile_Card.h"
#include "CPP_CombatStatics.h"
#include "CPP_ProgressionInterface.h"

UCPP_Action_UseCard::UCPP_Action_UseCard()
{
	ActionTag = FGameplayTag::RequestGameplayTag("Ability.Active.UseCard");
	CooldownTime = 0.5f;
}

void UCPP_Action_UseCard::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	UCPP_DeckComponent* DeckComp = Instigator->FindComponentByClass<UCPP_DeckComponent>();
	UCPP_ActionComponent* ActionComp = GetOwningComponent();

	if (DeckComp && ActionComp)
	{
		FCombatCard DrawnCard;
		if (DeckComp->DrawCard(DrawMode, DrawnCard))
		{
			if (bSelfCastMode)
			{
				// self
				if (DrawnCard.SelfActionClass)
					ActionComp->ExecuteActionOnce(DrawnCard.SelfActionClass);
			}
			else
			{
				//throw
				if (DrawnCard.CardProjectileClass)
				{
					ACPP_Projectile* SpawnedProj = UCPP_CombatStatics::SpawnProjectile(
						Instigator,
						DrawnCard.CardProjectileClass,
						0.0f,                     
						DrawnCard.ThrowSpeed,      
						DrawnCard.CardCollisionRadius, 
						DrawnCard.VisualScale,
						{}
					);

					// Передаем "Полезную нагрузку" (Payload) в снаряд
					if (ACPP_Projectile_Card* CardProj = Cast<ACPP_Projectile_Card>(SpawnedProj))
					{
						CardProj->SetImpactAction(DrawnCard.ImpactActionClass);
					}
				}
			}

			//stats
			if (Instigator->GetClass()->ImplementsInterface(UCPP_ProgressionInterface::StaticClass()))
			{
				for (const FStatModifier& Mod : DrawnCard.StatModifiers)
				{
					if (Mod.StatTag.MatchesTag(FGameplayTag::RequestGameplayTag("Resource")))
						ICPP_ProgressionInterface::Execute_AddResource(Instigator, Mod.StatTag, Mod.Value);
					else
						ICPP_ProgressionInterface::Execute_ModifyStat(Instigator, Mod);
				}
			}
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("deck empty"));
		}
	}

	//action wrapper finalizes immideatly
	StopAction(Instigator);
}
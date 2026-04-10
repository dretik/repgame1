#include "CPP_ActionComponent.h"
#include "CPP_Action.h"
#include "CPP_ActionSet.h"

UCPP_ActionComponent::UCPP_ActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCPP_ActionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ActionSet)
	{
		for (TSubclassOf<UCPP_Action> ActionClass : ActionSet->Actions)
		{
			if (ActionClass)
			{
				UCPP_Action* DefaultObj = ActionClass->GetDefaultObject<UCPP_Action>();
				if (DefaultObj && DefaultObj->bAutoUnlock)
				{
					AddAction(ActionClass);
				}
			}
		}
	}
}

void UCPP_ActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//cooldown or effects reset could be added
}

void UCPP_ActionComponent::AddAction(TSubclassOf<UCPP_Action> ActionClass)
{
	if (!ActionClass) return;

	// creating Action object. Outer = this
	UCPP_Action* NewAction = NewObject<UCPP_Action>(this, ActionClass);
	if (NewAction)
	{
		Actions.Add(NewAction);

		if (NewAction->bAutoStart && NewAction->CanStart(GetOwner()))
		{
			NewAction->StartAction(GetOwner());
		}
	}
}

bool UCPP_ActionComponent::StartActionByName(AActor* Instigator, FGameplayTag ActionTag)
{
	for (UCPP_Action* Action : Actions)
	{
		if (Action && Action->ActionTag == ActionTag)
		{
			if (!Action->CanStart(Instigator))
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Failed to start: %s (Cooldown or blocked)"), *ActionTag.ToString()));
				return false;
			}

			Action->StartAction(Instigator);
			return true;
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("Error: Action [%s] NOT FOUND in Component on Actor [%s]!"),
				*ActionTag.ToString(), *GetOwner()->GetName()));
	}
	return false;
}

bool UCPP_ActionComponent::StopActionByName(AActor* Instigator, FGameplayTag ActionTag)
{
	for (UCPP_Action* Action : Actions)
	{
		if (Action && Action->ActionTag == ActionTag)
		{
			if (Action->IsRunning())
			{
				Action->StopAction(Instigator);
				return true;
			}
		}
	}
	return false;
}

bool UCPP_ActionComponent::HasAnyMatchingTag(FGameplayTagContainer TagContainer) const
{
	return ActiveGameplayTags.HasAny(TagContainer);
}

UCPP_Action* UCPP_ActionComponent::GetAction(FGameplayTag ActionTag) const
{
	for (UCPP_Action* Action : Actions)
	{
		if (Action && Action->ActionTag == ActionTag)
		{
			return Action;
		}
	}
	return nullptr;
}
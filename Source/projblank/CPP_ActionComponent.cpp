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

	// Создаем объекты из классов по умолчанию
	if (ActionSet){
		for (TSubclassOf<UCPP_Action> ActionClass : ActionSet->Actions)
		{
			AddAction(ActionClass);
		}
	}
}

void UCPP_ActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Здесь можно добавить обновление кулдаунов или эффектов
}

void UCPP_ActionComponent::AddAction(TSubclassOf<UCPP_Action> ActionClass)
{
	if (!ActionClass) return;

	// Создаем объект Action. Важно: Outer = this (компонент)
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
				continue;
			}

			Action->StartAction(Instigator);
			return true;
		}
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
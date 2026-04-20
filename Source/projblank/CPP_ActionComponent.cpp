#include "CPP_ActionComponent.h"
#include "CPP_Action.h"
#include "CPP_ActionSet.h"
#include "AbilityCardData.h"
#include "CPP_AttributeComponent.h"
#include "CPP_ProgressionInterface.h"

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

EActionGrantResult UCPP_ActionComponent::GrantAction(TSubclassOf<UCPP_Action> ActionClass)
{
	if (!ActionClass) return EActionGrantResult::Failed;

	UCPP_Action* DefaultAction = ActionClass->GetDefaultObject<UCPP_Action>();
	if (!DefaultAction) return EActionGrantResult::Failed;

	FGameplayTag ActionTag = DefaultAction->ActionTag;
	int32 MaxAllowedLevel = DefaultAction->MaxLevel;

	int32 CurrentLevel = GetActionLevel(ActionTag);

	if (CurrentLevel >= MaxAllowedLevel)
	{
		return EActionGrantResult::MaxLevelReached;
	}
	if (CurrentLevel == 0)
	{
		AddAction(ActionClass);
		ActionLevels.Add(ActionTag, 1);
		return EActionGrantResult::Unlocked;
	}
	else
	{
		ActionLevels.Add(ActionTag, CurrentLevel + 1);
		return EActionGrantResult::Upgraded;
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

int32 UCPP_ActionComponent::GetActionLevel(FGameplayTag ActionTag) const
{
	if (const int32* FoundLevel = ActionLevels.Find(ActionTag))
	{
		return *FoundLevel;
	}
	return 0;
}

void UCPP_ActionComponent::SetActionLevel(FGameplayTag ActionTag, int32 NewLevel)
{
	ActionLevels.Add(ActionTag, NewLevel);
}

void UCPP_ActionComponent::RestoreActionLevels(const TMap<FGameplayTag, int32>& LoadedLevels)
{
	ActionLevels = LoadedLevels;

	if (ActionSet)
	{
		for (TSubclassOf<UCPP_Action> ActionClass : ActionSet->Actions)
		{
			if (ActionClass)
			{
				UCPP_Action* DefaultObj = ActionClass->GetDefaultObject<UCPP_Action>();

				if (DefaultObj && GetActionLevel(DefaultObj->ActionTag) > 0)
				{
					if (!GetAction(DefaultObj->ActionTag))
					{
						AddAction(ActionClass);
					}
				}
			}
		}
	}
}

void UCPP_ActionComponent::ApplyStatusEffect(TSubclassOf<UCPP_Action> ActionClass, AActor* Instigator)
{
	if (!ActionClass) return;

	UCPP_Action* DefaultObj = ActionClass->GetDefaultObject<UCPP_Action>();
	if (!DefaultObj) return;

	UCPP_Action* ExistingAction = GetAction(DefaultObj->ActionTag);

	if (ExistingAction)
	{
		if (ExistingAction->IsRunning())
		{
			ExistingAction->RefreshAction(Instigator);
		}
		else
		{
			ExistingAction->StartAction(Instigator);
		}
		return;
	}

	UCPP_Action* NewAction = NewObject<UCPP_Action>(this, ActionClass);
	if (NewAction)
	{
		Actions.Add(NewAction);
		if (NewAction->CanStart(Instigator))
		{
			NewAction->StartAction(Instigator);
		}
	}
}

void UCPP_ActionComponent::EquipActionToSlot(FGameplayTag SlotTag, TSubclassOf<UCPP_Action> ActionClass)
{
	if (!ActionClass || !SlotTag.IsValid()) return;

	//getter to check if exists
	UCPP_Action* ActionObj = GetAction(ActionClass->GetDefaultObject<UCPP_Action>()->ActionTag);

	if (!ActionObj)
	{
		// if no action - creates one
		AddAction(ActionClass);
		ActionLevels.Add(ActionClass->GetDefaultObject<UCPP_Action>()->ActionTag, 1);
	}

	//link in map
	EquippedAbilities.Add(SlotTag, ActionClass);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
		FString::Printf(TEXT("Equipped %s to Slot %s"), *ActionClass->GetName(), *SlotTag.ToString()));
}

bool UCPP_ActionComponent::StartActionBySlot(AActor* Instigator, FGameplayTag SlotTag)
{
	TSubclassOf<UCPP_Action>* ActionClassPtr = EquippedAbilities.Find(SlotTag);
	if (ActionClassPtr && *ActionClassPtr)
	{
		FGameplayTag ActionTag = (*ActionClassPtr)->GetDefaultObject<UCPP_Action>()->ActionTag;
		return StartActionByName(Instigator, ActionTag);
	}
	return false;
}

int32 UCPP_ActionComponent::GetActionLevelInSlot(FGameplayTag SlotTag) const
{
	const TSubclassOf<UCPP_Action>* ActionClassPtr = EquippedAbilities.Find(SlotTag);
	if (ActionClassPtr && *ActionClassPtr)
	{
		FGameplayTag ActionTag = (*ActionClassPtr)->GetDefaultObject<UCPP_Action>()->ActionTag;
		return GetActionLevel(ActionTag);
	}
	return 0;
}

void UCPP_ActionComponent::ApplyCardEffect(FAbilityCard ChosenCard, FGameplayTag TargetSlot)
{
	AActor* Owner = GetOwner();

	switch (ChosenCard.CardType)
	{
	case ECardType::ActiveAbility:
		EquipActionToSlot(TargetSlot, ChosenCard.ActionClass);
		break;

	case ECardType::StatUpgrade:
		if (Owner->GetClass()->ImplementsInterface(UCPP_ProgressionInterface::StaticClass()))
		{
			FGameplayTag Tag = ChosenCard.StatTag;
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Resource")))
			{
				ICPP_ProgressionInterface::Execute_AddResource(Owner, Tag, ChosenCard.StatModifierValue);
			}
			else
			{
				// stat mod
				FStatModifier Mod;
				Mod.StatTag = Tag;
				Mod.Value = ChosenCard.StatModifierValue;
				Mod.bIsMultiplier = false;

				ICPP_ProgressionInterface::Execute_ModifyStat(Owner, Mod);
			}
		}
	}
}
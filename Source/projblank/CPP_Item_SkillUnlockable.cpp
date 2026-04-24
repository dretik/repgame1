#include "CPP_Item_SkillUnlockable.h"
#include "CPP_BaseCharacter.h"
#include "CPP_Action.h"
#include "AbilityCardData.h"
#include "CPP_ActionComponent.h"

ACPP_Item_SkillUnlockable::ACPP_Item_SkillUnlockable()
{
    bAutoPickup = false;
    bIsInventoryItem = false;

    ItemName = NSLOCTEXT("Items", "SkillBookName", "Skill Tome");
}

void ACPP_Item_SkillUnlockable::Interact_Implementation(AActor* Interactor)
{
    if (bIsPickedUp) return;
    UCPP_ActionComponent* ActionComp = Interactor->FindComponentByClass<UCPP_ActionComponent>();
    if (!ActionComp || !ActionClass) return;
    UCPP_Action* DefaultAction = ActionClass->GetDefaultObject<UCPP_Action>();
    FGameplayTag Tag = DefaultAction->ActionTag;

    if (ActionComp->GetActionLevel(DefaultAction->ActionTag) >= DefaultAction->MaxLevel) return;

    // virtual card onflight
    FAbilityCard DummyCard;
    DummyCard.ActionClass = ActionClass;
    //only abiltiy

    //letting through pipeline
    bool bSuccess = ActionComp->ApplyCardEffect(DummyCard, FGameplayTag::EmptyTag);

    if (bSuccess)
    {
        bIsPickedUp = true;
        Super::Interact_Implementation(Interactor); //destroy
    }
}
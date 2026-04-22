#include "CPP_Item_SkillUnlockable.h"
#include "CPP_BaseCharacter.h"
#include "CPP_Action.h"
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

    //level check
    if (ActionComp->GetActionLevel(Tag) >= DefaultAction->MaxLevel)
    {
        //for notif use interface
        return;
    }

    if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag("Ability.Passive")))
    {
        ActionComp->GrantAction(ActionClass);
    }
    else
    {
        FGameplayTag TargetSlot = ActionComp->GetFirstEmptySlot();
        //for active from world
        // could be looking for first free slot
        ActionComp->EquipActionToSlot(TargetSlot, ActionClass);
    }

    bIsPickedUp = true;
    Super::Interact_Implementation(Interactor);
}
#include "CPP_Item_SkillUnlockable.h"
#include "CPP_BaseCharacter.h"
#include "CPP_PlayerCharacter.h"
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
    ACPP_PlayerCharacter* Player = Cast<ACPP_PlayerCharacter>(Interactor);
    if (!Player || !ActionClass) return;

    UCPP_Action* DefaultAction = ActionClass->GetDefaultObject<UCPP_Action>();
    if (!DefaultAction) return;
    UCPP_ActionComponent* ActionComp = Player->FindComponentByClass<UCPP_ActionComponent>();
    if (!ActionComp) return;

    //if level of ability is maxed - shownotif or addgold or smth
    if (ActionComp->GetActionLevel(DefaultAction->ActionTag) >= DefaultAction->MaxLevel)
    {
        Player->ShowNotification(NSLOCTEXT("Abilities", "MaxLevel", "Skill already at maximum level!"), FLinearColor::Red);
        return; // item remains
    }

    bool bSuccess = Player->GrantAbility(ActionClass);

    if (bSuccess)
    {
        bIsPickedUp = true;
        // calling super to exec:
        // statmodifiers -> notification -> destroy
        Super::Interact_Implementation(Interactor);
    }
}
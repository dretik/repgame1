#include "CPP_Item_SkillUnlockable.h"
#include "CPP_BaseCharacter.h"
#include "CPP_PlayerCharacter.h"
#include "CPP_Action.h"

ACPP_Item_SkillUnlockable::ACPP_Item_SkillUnlockable()
{
    bAutoPickup = false;
    bIsInventoryItem = false;

    ItemName = NSLOCTEXT("Items", "SkillBookName", "Skill Tome");
}

void ACPP_Item_SkillUnlockable::Interact_Implementation(AActor* Interactor)
{
    ACPP_PlayerCharacter* BaseChar = Cast<ACPP_PlayerCharacter>(Interactor);

    if (BaseChar && ActionClass)
    {
        bool bSuccess = BaseChar->GrantAbility(ActionClass);

        if (bSuccess)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, TEXT("Skill Item Used!"));
        }
    }

    // calling super to exec:
    // statmodifiers -> notification -> destroy
    Super::Interact_Implementation(Interactor);
}
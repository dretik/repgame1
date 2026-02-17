#include "CPP_Item_SkillUnlockable.h"
#include "CPP_BaseCharacter.h"
#include "CPP_Action.h"

ACPP_Item_SkillUnlockable::ACPP_Item_SkillUnlockable()
{
    // Настраиваем дефолты
    bAutoPickup = false; // Книги обычно не подбираются сами
    bIsInventoryItem = false; // Обычно книга используется сразу, а не кладется в инвентарь

    // Имя по умолчанию
    ItemName = NSLOCTEXT("Items", "SkillBookName", "Skill Tome");
}

void ACPP_Item_SkillUnlockable::Interact_Implementation(AActor* Interactor)
{
    ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Interactor);

    if (BaseChar && ActionClass)
    {
        // Вызываем НОВУЮ версию GrantAbility, которая принимает класс
        bool bSuccess = BaseChar->GrantAbility(ActionClass);

        if (bSuccess)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, TEXT("Skill Item Used!"));
        }
    }

    // Вызываем Super, чтобы сработала остальная логика BaseItem:
    // 1. Применились StatModifiers (если есть)
    // 2. Показалось уведомление
    // 3. Предмет уничтожился (Destroy)
    Super::Interact_Implementation(Interactor);
}
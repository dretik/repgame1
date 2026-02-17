// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_BaseItem.h"
#include "CPP_Item_SkillUnlockable.generated.h"

/**
 * 
 */
UCLASS()
class PROJBLANK_API ACPP_Item_SkillUnlockable : public ACPP_BaseItem
{
	GENERATED_BODY()
	
public:
    ACPP_Item_SkillUnlockable();

    virtual void Interact_Implementation(AActor* Interactor) override;
    TSubclassOf<UCPP_Action> GetActionClass() const { return ActionClass; }

    const int32 GlobalMaxSkillLevel = 3;

protected:
    // Класс способности (например, BP_Action_Fireball)
    // Раньше тут был тег, теперь полноценный класс
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
        TSubclassOf<UCPP_Action> ActionClass;
};

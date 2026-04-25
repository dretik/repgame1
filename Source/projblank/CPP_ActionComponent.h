#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilityCardData.h"
#include "CPP_ActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySwapRequired, FAbilityCard, PendingCard);

class UCPP_Action;
UENUM(BlueprintType)
enum class EActionGrantResult : uint8
{
	Failed,
	Unlocked,
	Upgraded,
	MaxLevelReached
};
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJBLANK_API UCPP_ActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPP_ActionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
		FGameplayTagContainer ActiveGameplayTags;

	UFUNCTION(BlueprintCallable, Category = "Actions")
		void AddAction(TSubclassOf<UCPP_Action> ActionClass);
	UFUNCTION(BlueprintCallable, Category = "Actions|Levels")
		EActionGrantResult GrantAction(TSubclassOf<UCPP_Action> ActionClass);
	UFUNCTION(BlueprintCallable, Category = "Actions")
		UCPP_Action* GetAction(FGameplayTag ActionTag) const;

	UFUNCTION(BlueprintCallable, Category = "Actions")
		bool StartActionByName(AActor* Instigator, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
		bool StopActionByName(AActor* Instigator, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
		bool HasAnyMatchingTag(FGameplayTagContainer TagContainer) const;

	class UCPP_ActionSet* GetActionSet() const { return ActionSet; }

	UFUNCTION(BlueprintCallable, Category = "Actions")
		int32 GetActionLevel(FGameplayTag ActionTag) const;

	UFUNCTION(BlueprintCallable, Category = "Actions")
		void SetActionLevel(FGameplayTag ActionTag, int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Actions|Effects")
		void ApplyStatusEffect(TSubclassOf<UCPP_Action> ActionClass, AActor* Instigator);

	const TMap<FGameplayTag, int32>& GetAllActionLevels() const { return ActionLevels; }

	void RestoreActionLevels(const TMap<FGameplayTag, int32>& LoadedLevels);

	//to action slot or will replace
	UFUNCTION(BlueprintCallable, Category = "Actions|Slots")
		bool EquipActionToSlot(FGameplayTag SlotTag, FAbilityCard PendingCard);

	const TMap<FGameplayTag, TSubclassOf<UCPP_Action>>& GetEquippedAbilities() const 
	{ return EquippedAbilities; }

	//starts slot action
	UFUNCTION(BlueprintCallable, Category = "Actions|Slots")
		bool StartActionBySlot(AActor* Instigator, FGameplayTag SlotTag);

	//slot action level
	UFUNCTION(BlueprintCallable, Category = "Actions|Slots")
		int32 GetActionLevelInSlot(FGameplayTag SlotTag) const;
	//event if no slot is available
	UPROPERTY(BlueprintAssignable, Category = "Actions|Events")
		FOnAbilitySwapRequired OnAbilitySwapRequired;
	//called from ui 
	UFUNCTION(BlueprintCallable, Category = "Roguelike")
		bool ApplyCardEffect(FAbilityCard ChosenCard, FGameplayTag TargetSlot);

	//returns first available ability slot
	UFUNCTION(BlueprintCallable, Category = "Actions|Slots")
		FGameplayTag GetFirstEmptySlot() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Actions")
		class UCPP_ActionSet* ActionSet;

	UPROPERTY(BlueprintReadOnly, Category = "Actions")
		TArray<UCPP_Action*> Actions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions|Levels")
		TMap<FGameplayTag, int32> ActionLevels;

	//slot abilities map (slottag -> actionclass)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions|Slots")
		TMap<FGameplayTag, TSubclassOf<UCPP_Action>> EquippedAbilities;

	UPROPERTY(EditAnywhere, Category = "Roguelike")
		class UAbilityCardPool* CardPool;

	//// card that waits for confirmation
	//FAbilityCard PendingCard;

	//// pending flag
	//bool bHasPendingCard = false;

	////auxilary to apply stats only (for transaction purposes)
	//void ApplyStatsFromCard(const FAbilityCard& Card);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "RarityTypes.h"
#include "CPP_Action.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStateChanged, class UCPP_Action*, Action);

class UWorld;
class UCPP_ActionComponent;

USTRUCT(BlueprintType)
struct FActionRepData
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly)
		bool bIsRunning;

	UPROPERTY(BlueprintReadOnly)
		class AActor* Instigator;
};

UCLASS(Blueprintable)
class PROJBLANK_API UCPP_Action : public UObject
{
	GENERATED_BODY()

public:
	//delegate
	UPROPERTY(BlueprintAssignable)
		FOnActionStateChanged OnActionStopped;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
		FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
		float CooldownTime = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Action")
		float GetCooldownRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
		float GetCooldownNormalized() const; //0 1 for progressbar

	UFUNCTION(BlueprintCallable, Category = "Action")
		virtual float GetActionDamageMultiplier() const;

	UPROPERTY(EditDefaultsOnly, Category = "Action", meta = (ClampMin = "0.01", UIMin = "0.01"))
		float Duration = 1.0f;
	//fallsafe const
	static constexpr float MinActionDuration = 0.01f;

	//autostart (for passives?)
	UPROPERTY(EditDefaultsOnly, Category = "Action")
		bool bAutoStart = false;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		void StopAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		void RefreshAction(AActor* Instigator);

	UPROPERTY(EditDefaultsOnly, Category = "Action")
		bool bAutoUnlock = false;

	UPROPERTY(EditDefaultsOnly, Category = "Action", meta = (ClampMin = "1"))
		int32 MaxLevel = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		class UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (MultiLine = true))
		FText ActionDescription;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI")
		FText GetFormattedDescription();

	UFUNCTION(BlueprintPure, Category = "Action|Utility", meta = (DeterminesOutputType = "ActionClass"))
		static UCPP_Action* GetActionCDO(TSubclassOf<UCPP_Action> ActionClass);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		ECardRarity Rarity;

	UFUNCTION(BlueprintCallable, Category = "Action")
		void SetOverrideLocation(FVector NewLocation);
	UFUNCTION(BlueprintCallable, Category = "Action")
		void ClearOverrideLocation() { bUseOverrideLocation = false; }

	UFUNCTION(BlueprintCallable, Category = "Action|Targeting")
		bool IsValidTarget(AActor* Target, AActor* Instigator) const;

	//for timers
	virtual UWorld* GetWorld() const override;

	bool IsRunning() const;

protected:
	UFUNCTION(BlueprintCallable, Category = "Action")
		UCPP_ActionComponent* GetOwningComponent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
		FActionRepData RepData;

	UPROPERTY(VisibleAnywhere, Category = "Action")
		float TimeStarted;

	UFUNCTION(BlueprintCallable, Category = "Action")
		FVector GetActionLocation() const;

	UPROPERTY(BlueprintReadWrite, Category = "Action")
		FVector OverrideLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Action")
		bool bUseOverrideLocation = false;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Targeting")
		bool bAffectEnemies = true;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Targeting")
		bool bAffectSelf = false;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Targeting")
		bool bAffectFriends = false;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
		FTimerHandle TimerHandle_AutoStop;
};
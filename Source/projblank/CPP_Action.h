#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
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
	UPROPERTY(EditDefaultsOnly, Category = "Action")
		FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
		float CooldownTime = 1.0f;

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

	UPROPERTY(BlueprintReadOnly, Category = "Action")
		FTimerHandle TimerHandle_AutoStop;
};
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "CPP_Action.generated.h"

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

/**
 * ability base class
 */
UCLASS(Blueprintable)
class PROJBLANK_API UCPP_Action : public UObject
{
	GENERATED_BODY()

public:
	// Тег для запуска (например, "Ability.Fireball")
	UPROPERTY(EditDefaultsOnly, Category = "Action")
		FGameplayTag ActionTag;

	// Время перезарядки
	UPROPERTY(EditDefaultsOnly, Category = "Action")
		float CooldownTime = 1.0f;

	// Авто-запуск при получении компонентом (пассивки)
	UPROPERTY(EditDefaultsOnly, Category = "Action")
		bool bAutoStart = false;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
		void StopAction(AActor* Instigator);

	// Нужно для работы таймеров и спавна акторов внутри UObject
	virtual UWorld* GetWorld() const override;

	bool IsRunning() const;

protected:
	UFUNCTION(BlueprintCallable, Category = "Action")
		UCPP_ActionComponent* GetOwningComponent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
		FActionRepData RepData;

	UPROPERTY(VisibleAnywhere, Category = "Action")
		float TimeStarted;
};
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CPP_ActionComponent.generated.h"

class UCPP_Action;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJBLANK_API UCPP_ActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPP_ActionComponent();

	// Теги, которые сейчас активны (например, "Ability.Casting", "State.Stunned")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
		FGameplayTagContainer ActiveGameplayTags;

	UFUNCTION(BlueprintCallable, Category = "Actions")
		void AddAction(TSubclassOf<UCPP_Action> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
		bool StartActionByName(AActor* Instigator, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
		bool StopActionByName(AActor* Instigator, FGameplayTag ActionTag);

protected:
	virtual void BeginPlay() override;

	// Список дефолтных способностей (задается в редакторе)
	UPROPERTY(EditAnywhere, Category = "Actions")
		TArray<TSubclassOf<UCPP_Action>> DefaultActions;

	// Созданные объекты способностей
	UPROPERTY(BlueprintReadOnly, Category = "Actions")
		TArray<UCPP_Action*> Actions;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
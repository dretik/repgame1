#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_Projectile.generated.h"

class ACPP_Projectile;
class UCPP_Action_Effect;
class UPaperFlipbook;

USTRUCT(BlueprintType)
struct FProjectileLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "LevelData")
		TSubclassOf<ACPP_Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "LevelData")
		float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "LevelData")
		float ProjectileScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "LevelData")
		TArray<TSubclassOf<UCPP_Action_Effect>> ImpactEffects;

	// projectile amount or speed could be added
};

UCLASS()
class PROJBLANK_API UCPP_Action_Projectile : public UCPP_Action
{
	GENERATED_BODY()

public:
	UCPP_Action_Projectile();

	virtual void StartAction_Implementation(AActor* Instigator) override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		float GetCurrentDamage() const;

	virtual FText GetFormattedDescription_Implementation() override;
protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
		TArray<FProjectileLevelData> LevelConfigs;

	UPROPERTY(EditAnywhere, Category = "Visuals")
		UPaperFlipbook* CastAnim;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackAnimDelay = 0.2f;

	UFUNCTION()
		void AttackDelay_Elapsed(ACharacter* InstigatorCharacter);
};
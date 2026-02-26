#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_Projectile.generated.h"

class ACPP_Projectile;
class UPaperFlipbook;

UCLASS()
class PROJBLANK_API UCPP_Action_Projectile : public UCPP_Action
{
	GENERATED_BODY()

public:
	UCPP_Action_Projectile();

	virtual void StartAction_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
		TSubclassOf<ACPP_Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Visuals")
		UPaperFlipbook* CastAnim;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackAnimDelay = 0.2f;

	UFUNCTION()
		void AttackDelay_Elapsed(ACharacter* InstigatorCharacter);
};
#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_Projectile.generated.h"

class ACPP_Projectile;
class UPaperFlipbook;

/**
 * Действие: Спавн снаряда (Файербол)
 */
UCLASS()
class PROJBLANK_API UCPP_Action_Projectile : public UCPP_Action
{
	GENERATED_BODY()

public:
	UCPP_Action_Projectile();

	virtual void StartAction_Implementation(AActor* Instigator) override;

protected:
	// Класс снаряда
	UPROPERTY(EditAnywhere, Category = "Combat")
		TSubclassOf<ACPP_Projectile> ProjectileClass;

	// Анимация каста (необязательно, но полезно)
	UPROPERTY(EditAnywhere, Category = "Visuals")
		UPaperFlipbook* CastAnim;

	// Задержка перед спавном (чтобы совпало с анимацией руки)
	UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackAnimDelay = 0.2f;

	// Вспомогательная функция для таймера
	UFUNCTION()
		void AttackDelay_Elapsed(ACharacter* InstigatorCharacter);
};
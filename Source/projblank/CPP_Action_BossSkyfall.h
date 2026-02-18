// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPP_Action.h"
#include "CPP_Action_BossSkyfall.generated.h"

class ACPP_Projectile;
class UPaperFlipbook;

UCLASS()
class PROJBLANK_API UCPP_Action_BossSkyfall : public UCPP_Action
{
	GENERATED_BODY()

public:
	UCPP_Action_BossSkyfall();

	virtual void StartAction_Implementation(AActor* Instigator) override;
	virtual void StopAction_Implementation(AActor* Instigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skyfall")
		TSubclassOf<ACPP_Projectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skyfall | Config")
		int32 MeteorsCount = 80;

	UPROPERTY(EditDefaultsOnly, Category = "Skyfall | Config")
		float SpawnRate = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Skyfall | Config")
		float DamagePerMeteor = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skyfall | Config")
		float SpawnHeight = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skyfall | Config")
		float SpawnRadius = 300.0f;

	UFUNCTION()
		void SwitchToLoopAnim(AActor* Instigator);

	UPROPERTY(EditDefaultsOnly, Category = "Skyfall | Visuals")
		class UPaperFlipbook* IntroAnim;
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
		class UPaperFlipbook* LoopAnim;

	FTimerHandle TimerHandle_Spawn;
	int32 MeteorsSpawned;

	UFUNCTION()
		void SpawnMeteor(AActor* Instigator);
};
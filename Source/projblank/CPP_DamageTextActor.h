// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_DamageTextActor.generated.h"

class UWidgetComponent;

UCLASS()
class PROJBLANK_API ACPP_DamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_DamageTextActor();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateDamageText(float DamageAmount);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UWidgetComponent* DamageWidgetComp;

	// Время жизни текста перед уничтожением
	UPROPERTY(EditDefaultsOnly, Category = "Config")
		float LifeTime = 1.5f;

	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

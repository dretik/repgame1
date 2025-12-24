// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "CPP_EnemySpawner.generated.h"

class UBoxComponent;
class UWidgetComponent;
class UArrowComponent;

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<AActor> EnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
        float SpawnWeight = 1.0f;
};

UCLASS()
class PROJBLANK_API ACPP_EnemySpawner : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    ACPP_EnemySpawner();

protected:
    virtual void BeginPlay() override;

    // components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UStaticMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UWidgetComponent* PromptWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UArrowComponent* SpawnPoint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
        TArray<FEnemySpawnInfo> SpawnList;

public:
    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION()
        void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
        void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

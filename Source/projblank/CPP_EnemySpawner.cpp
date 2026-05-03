// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "CPP_BaseCharacter.h"
#include "CPP_BaseEnemy.h" 

// Sets default values
ACPP_EnemySpawner::ACPP_EnemySpawner()
{
    // 1.
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    // 2. trigger
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
    TriggerBox->SetCollisionProfileName("Trigger");

    // 3. widget
    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen); //facing camera 
    PromptWidget->SetDrawAtDesiredSize(true);
    PromptWidget->SetVisibility(false);

    // 4. spawn point
    SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint"));
    SpawnPoint->SetupAttachment(RootComponent);
    SpawnPoint->SetRelativeLocation(FVector(150.f, 0.f, 0.f));
}

// Called when the game starts or when spawned
void ACPP_EnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// sub to overlap events
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACPP_EnemySpawner::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ACPP_EnemySpawner::OnOverlapEnd);

    if (bAutoSpawn && SpawnInterval > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_AutoSpawn, this, &ACPP_EnemySpawner::ExecuteSpawn, SpawnInterval, true, InitialDelay);
    }
}

//when approaching
void ACPP_EnemySpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACPP_BaseCharacter::StaticClass()))
    {
        PromptWidget->SetVisibility(true);
    }
}

//when leaving
void ACPP_EnemySpawner::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACPP_BaseCharacter::StaticClass()))
    {
        PromptWidget->SetVisibility(false);
    }
}

void ACPP_EnemySpawner::Interact_Implementation(AActor* Interactor)
{
    if (!bAutoSpawn)
    {
        ExecuteSpawn();
    }
}

void ACPP_EnemySpawner::ExecuteSpawn()
{
    if (EnemiesSpawnedTotal >= MaxTotalEnemies)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Spawner Depleted"));

        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AutoSpawn);
        PromptWidget->SetVisibility(false);
        return;
    }

    int32 CurrentGroupSize = FMath::RandRange(MinGroupSize, MaxGroupSize);

    for (int32 i = 0; i < CurrentGroupSize; i++)
    {
        if (EnemiesSpawnedTotal >= MaxTotalEnemies) break;

        TSubclassOf<AActor> SelectedClass = GetRandomEnemyClassFromList();
        if (SelectedClass)
        {
            FVector SpawnLocation = GetRandomSpawnLocation();

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
                SelectedClass,
                SpawnLocation,
                GetActorRotation(),
                SpawnParams
                );

            if (SpawnedActor)
            {
                EnemiesSpawnedTotal++;
                SpawnedActor->Tags.Add(FName("Dynamic"));

                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
                    FString::Printf(TEXT("Spawned %d/%d: %s"), EnemiesSpawnedTotal, MaxTotalEnemies, *SelectedClass->GetName()));
            }
        }
    }
}

TSubclassOf<AActor> ACPP_EnemySpawner::GetRandomEnemyClassFromList()
{
    if (SpawnList.Num() == 0) return nullptr;

    float TotalWeight = 0.0f;
    for (const FEnemySpawnInfo& Entry : SpawnList)
    {
        if (Entry.EnemyClass) TotalWeight += Entry.SpawnWeight;
    }

    if (TotalWeight <= 0.0f) return nullptr;

    float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;

    for (const FEnemySpawnInfo& Entry : SpawnList)
    {
        if (!Entry.EnemyClass) continue;
        CurrentSum += Entry.SpawnWeight;
        if (RandomPoint <= CurrentSum)
        {
            return Entry.EnemyClass;
        }
    }
    return nullptr;
}

FVector ACPP_EnemySpawner::GetRandomSpawnLocation() const
{
    FVector Origin = GetActorLocation();

    float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
    float Distance = bSpawnOnCircleEdge ? SpawnRadius : FMath::FRandRange(0.0f, SpawnRadius);

    FVector Offset;
    Offset.X = FMath::Cos(Angle) * Distance;
    Offset.Y = FMath::Sin(Angle) * Distance;
    Offset.Z = 0.0f;

    return Origin + Offset;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_EnemySpawner.h"
#include "Kismet/GameplayStatics.h"
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
    TriggerBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

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
}

//when approaching
void ACPP_EnemySpawner::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
    {
        bPlayerInside = true;

        if (!bAutoSpawn)
        PromptWidget->SetVisibility(true);

        //player enters
        if (bAutoSpawn)
        {
            //ExecuteSpawn();

            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle_AutoSpawn,
                this,
                &ACPP_EnemySpawner::ExecuteSpawn,
                SpawnInterval,
                true,
                InitialDelay
            );
        }
    }
}

//when leaving
void ACPP_EnemySpawner::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
    {
        bPlayerInside = false;
        if (!bAutoSpawn)
        PromptWidget->SetVisibility(false);

        //player leaves
        if (bAutoSpawn)
        {
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AutoSpawn);
        }
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
    AliveEnemies.RemoveAll([](AActor* A) {
        if (!A || !IsValid(A)) return true;
        ACPP_BaseCharacter* Char = Cast<ACPP_BaseCharacter>(A);
        return Char && Char->IsDead();
        });

    UE_LOG(LogTemp, Warning, TEXT("Spawner %s checks: Alive: %d / Max: %d TotalSpawned=%d/%d"), *GetName(), AliveEnemies.Num(), MaxActiveEnemiesFromThisSpawner, EnemiesSpawnedTotal, MaxTotalEnemies);

    if (AliveEnemies.Num() >= MaxActiveEnemiesFromThisSpawner)
    {
        return;
    }

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
        if (AliveEnemies.Num() >= MaxActiveEnemiesFromThisSpawner) break;

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
                AliveEnemies.Add(SpawnedActor); //tracking array

                ACPP_BaseCharacter* EnemyChar = Cast<ACPP_BaseCharacter>(SpawnedActor);
                if (EnemyChar)
                {
                    EnemyChar->OnCharacterDeath.AddDynamic(this, &ACPP_EnemySpawner::HandleEnemyDeath);
                }

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

void ACPP_EnemySpawner::HandleEnemyDeath(AActor* DeadEnemy)
{
    if (DeadEnemy)
    {
        AliveEnemies.Remove(DeadEnemy);
        UE_LOG(LogTemp, Warning, TEXT("Enemy Died. Remaining in list: %d"), AliveEnemies.Num());
    }
}
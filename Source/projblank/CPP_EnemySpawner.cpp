// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "CPP_BaseCharacter.h"

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
    SpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
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
    if (EnemyClassToSpawn)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        GetWorld()->SpawnActor<AActor>(
            EnemyClassToSpawn,
            SpawnPoint->GetComponentLocation(),
            SpawnPoint->GetComponentRotation(),
            SpawnParams
            );

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Enemy Spawned!"));
    }
}
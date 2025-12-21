// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseItem.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"

ACPP_BaseItem::ACPP_BaseItem()
{
    // vollision
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    RootComponent = SphereComp;

    // physics
    SphereComp->SetSimulatePhysics(true);
    SphereComp->SetSphereRadius(16.0f);
    SphereComp->SetCollisionProfileName("PhysicsActor");
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // sprite
    ItemSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("ItemSprite"));
    ItemSprite->SetupAttachment(RootComponent);
    ItemSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SphereComp->SetLinearDamping(1.0f);
    SphereComp->SetAngularDamping(3.0f);
    SphereComp->GetBodyInstance()->bLockXRotation = true;
    SphereComp->GetBodyInstance()->bLockYRotation = true;
    SphereComp->GetBodyInstance()->bLockZRotation = true;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(80.0f);

    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore); 
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void ACPP_BaseItem::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_BaseItem::Interact_Implementation(AActor* Interactor)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Picked up: %s"), *ItemName.ToString()));
    }

    // inventory interaction logic to be added

    Destroy();
}


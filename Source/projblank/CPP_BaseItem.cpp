// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseItem.h"
#include "CPP_BaseCharacter.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"

ACPP_BaseItem::ACPP_BaseItem()
{
    // collision
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    RootComponent = SphereComp;

    // physics
    SphereComp->SetSimulatePhysics(true);
    SphereComp->SetSphereRadius(16.0f);
    SphereComp->SetCollisionProfileName("PhysicsActor");
    SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    SphereComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
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

    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACPP_BaseItem::OnOverlapBegin);
    }

    FTimerHandle EnableTimer;
    GetWorldTimerManager().SetTimer(EnableTimer, this, &ACPP_BaseItem::EnablePhysicsCollision, 0.5f, false);

}

void ACPP_BaseItem::Interact_Implementation(AActor* Interactor)
{
    ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Interactor);

    if (BaseChar)
    {

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Picked up: %s"), *ItemName.ToString()));

        for (const FStatModifier& Mod : StatModifiers)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ITEM: Calling ApplyStatModifier..."));

            BaseChar->ApplyStatModifier(Mod);
        }

        Destroy();
    }
}

void ACPP_BaseItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bAutoPickup) return;

    ACPP_BaseCharacter* OverlappedCharacter = Cast<ACPP_BaseCharacter>(OtherActor);

    if (OverlappedCharacter && OverlappedCharacter->IsPlayerControlled())
    {
        Interact_Implementation(OverlappedCharacter);
        // for more complex interface logic
        // IInteractableInterface::Execute_Interact(this, OtherActor);
    }
}

void ACPP_BaseItem::EnablePhysicsCollision()
{
    if (InteractionSphere)
    {
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACPP_BaseItem::OnOverlapBegin);
    }

    if (SphereComp)
    {
        SphereComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
        SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

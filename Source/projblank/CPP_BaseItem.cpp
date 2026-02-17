// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseItem.h"
#include "CPP_BaseCharacter.h"
#include "Components/SphereComponent.h"
#include "CPP_InventoryComponent.h" 
#include "PaperSpriteComponent.h"
#include "Components/WidgetComponent.h"
#include "CPP_SaveGame.h" 
#include "CPP_GameInstance.h" 

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

    //labelwidget
    LabelWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LabelWidget"));
    LabelWidget->SetupAttachment(RootComponent);

    LabelWidget->SetWidgetSpace(EWidgetSpace::Screen);
    LabelWidget->SetDrawAtDesiredSize(true);
    LabelWidget->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
    LabelWidget->SetVisibility(false);

    ItemName = NSLOCTEXT("Items", "DefaultItemName", "Unknown Item");
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

    if (!ItemName.IsEmpty())
    {
        LabelWidget->SetVisibility(true);

        UUserWidget* WidgetObj = LabelWidget->GetUserWidgetObject();
        if (WidgetObj)
        {

            FText NameText = ItemName;

            FProperty* Prop = WidgetObj->GetClass()->FindPropertyByName(FName("LabelName"));
            if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
            {
                StrProp->SetPropertyValue_InContainer(WidgetObj, ItemName.ToString());
            }
            else if (FTextProperty* TxtProp = CastField<FTextProperty>(Prop))
            {
                TxtProp->SetPropertyValue_InContainer(WidgetObj, NameText);
            }
        }
    }

    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
    // Если мы загружаемся ИЛИ продолжаем играть
    if (GI)
    {
        // Сначала грузим список из файла, если это загрузка
        if (GI->bIsLoadingSave)
        {
            UCPP_SaveGame* LoadInst = Cast<UCPP_SaveGame>(UGameplayStatics::LoadGameFromSlot(GI->SaveSlotName, 0));
            if (LoadInst) GI->CurrentSessionCollectedItems = LoadInst->CollectedItems;
        }

        // Проверяем, не собрали ли меня
        if (GI->CurrentSessionCollectedItems.Contains(GetName()))
        {
            Destroy(); // Меня уже нет
            return;
        }
    }
}

void ACPP_BaseItem::Interact_Implementation(AActor* Interactor)
{
    ACPP_BaseCharacter* BaseChar = Cast<ACPP_BaseCharacter>(Interactor);

    if (BaseChar && BaseChar->IsPlayerControlled())
    {

        for (const FStatModifier& Mod : StatModifiers)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ITEM: Calling ApplyStatModifier..."));

            BaseChar->ApplyStatModifier(Mod);
        }

        if (bIsInventoryItem)
        {
            UCPP_InventoryComponent* InventoryComp = BaseChar->FindComponentByClass<UCPP_InventoryComponent>();
            if (InventoryComp)
            {
                InventoryComp->AddItem(this->GetClass(), 1);
            }
        }

        //if (AbilityToUnlock.IsValid())
        //{
        //    BaseChar->GrantAbility(AbilityToUnlock, MaxAbilityLevel);
        //}

        FText Msg = FText::Format(
            NSLOCTEXT("HUD", "Pickup", "Picked up: {0}"),
            ItemName
        );
        BaseChar->ShowNotification(Msg, FLinearColor::Green);

        UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
        if (GI)
        {
            GI->AddCollectedItem(GetName());
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

#include "CPP_VisualComponent.h"
#include "PaperFlipbookComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "CPP_VisualStatics.h"
#include "TimerManager.h"

UCPP_VisualComponent::UCPP_VisualComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCPP_VisualComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        MySprite = Owner->FindComponentByClass<UPaperFlipbookComponent>();
    }
}

void UCPP_VisualComponent::PlayHitFlash(float Duration, FLinearColor FlashColor)
{
    if (!MySprite) return;

    // ceating dynamic material 1 time
    if (!DynamicMaterial)
    {
        DynamicMaterial = MySprite->CreateDynamicMaterialInstance(0);
    }

    if (DynamicMaterial)
    {
        // material must contain "FlashAmount" and "FlashColor"
        DynamicMaterial->SetScalarParameterValue("FlashAmount", 1.0f);
        DynamicMaterial->SetVectorParameterValue("FlashColor", FlashColor);

        GetWorld()->GetTimerManager().SetTimer(TimerHandle_Flash, this, &UCPP_VisualComponent::OnFlashTimerExpired, Duration, false);
    }
}

void UCPP_VisualComponent::OnFlashTimerExpired()
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue("FlashAmount", 0.0f);
    }
}

void UCPP_VisualComponent::HandleDamageReceived(float DamageAmount, TSubclassOf<AActor> TextClass, UNiagaraSystem* HitFX)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    PlayHitFlash();

    UCPP_VisualStatics::SpawnNiagaraEffect(Owner, HitFX, Owner->GetActorLocation());

    UCPP_VisualStatics::SpawnDamageText(Owner, TextClass, DamageAmount, Owner->GetActorLocation());
}

void UCPP_VisualComponent::UpdateSpriteFacing(FVector Velocity, float BaseScale)
{
    if (!MySprite || Velocity.SizeSquared() < 0.01f) return;

    FVector NewScale = MySprite->GetRelativeScale3D();
    if (Velocity.Y > 0.1f) {
        NewScale.X = BaseScale;
    }
    else if (Velocity.Y < -0.1f) {
        NewScale.X = -BaseScale;
    }
    MySprite->SetRelativeScale3D(NewScale);
}

void UCPP_VisualComponent::HandleDeath(UNiagaraSystem* DeathFX)
{
    UCPP_VisualStatics::SpawnNiagaraEffect(GetOwner(), DeathFX, GetOwner()->GetActorLocation());
}
#include "CPP_VisualComponent.h"
#include "PaperFlipbookComponent.h"
#include "Components/MeshComponent.h"
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
        MeshComp = Owner->FindComponentByClass<UMeshComponent>();
    }
}

void UCPP_VisualComponent::PlayHitFlash(float Duration, FLinearColor FlashColor)
{
    if (!MeshComp) return;

    // ceating dynamic material 1 time
    if (!DynamicMaterial)
    {
        DynamicMaterial = MeshComp->CreateDynamicMaterialInstance(0);
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
    if (bFlippingLocked || !MeshComp || Velocity.SizeSquared() < 0.1f) return;

    float DesiredWorldDirY = (Velocity.Y > 0) ? 1.0f : -1.0f;

    ApplyFlipping(DesiredWorldDirY, BaseScale);
}

void UCPP_VisualComponent::HandleDeath(UNiagaraSystem* DeathFX)
{
    UCPP_VisualStatics::SpawnNiagaraEffect(GetOwner(), DeathFX, GetOwner()->GetActorLocation());
}

void UCPP_VisualComponent::LockFlipping(float Duration)
{
    bFlippingLocked = true;

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_FlipLock, this, &UCPP_VisualComponent::UnlockFlipping, Duration, false);
}

void UCPP_VisualComponent::FaceLocation(FVector TargetLocation, float BaseScale)
{
    if (!MeshComp) return;

    FVector OwnerLoc = GetOwner()->GetActorLocation();

    float DesiredWorldDirY = (TargetLocation.Y > OwnerLoc.Y) ? 1.0f : -1.0f;

    ApplyFlipping(DesiredWorldDirY, BaseScale);
}

void UCPP_VisualComponent::ApplyFlipping(float DesiredWorldDirY, float BaseScale)
{
    AActor* Owner = GetOwner();
    FVector NewScale = MeshComp->GetRelativeScale3D();

    float ActorForwardX = (Owner->GetActorForwardVector().X >= 0) ? 1.0f : -1.0f;

    float InversionMult = bInvertVisualFlipping ? -1.0f : 1.0f;

    float FinalScaleX = DesiredWorldDirY * ActorForwardX * FMath::Abs(BaseScale) * InversionMult;

    NewScale.X = FinalScaleX;
    MeshComp->SetRelativeScale3D(NewScale);
}
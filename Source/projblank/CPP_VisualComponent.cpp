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

        if (MeshComp)
        {
            DynamicMaterial = MeshComp->CreateDynamicMaterialInstance(0);

            ResetMaterialParameters();

            MeshComp->SetUsingAbsoluteRotation(true);
            //game is along y axis
            MeshComp->SetWorldRotation(FRotator(0, 90, 0));
        }
    }
}

void UCPP_VisualComponent::ResetMaterialParameters()
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue("FlashAmount", 0.0f);
        DynamicMaterial->SetScalarParameterValue("StatusIntensity", 0.0f);

        DynamicMaterial->SetScalarParameterValue("EmissiveBoost", 1.0f);

        DynamicMaterial->SetVectorParameterValue("FlashColor", FLinearColor::White);
        DynamicMaterial->SetVectorParameterValue("StatusColor", FLinearColor::Black);
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
        DynamicMaterial->SetScalarParameterValue("EmissiveBoost", 2.0f);
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_Flash, this, &UCPP_VisualComponent::OnFlashTimerExpired, Duration, false);
    }
}

void UCPP_VisualComponent::OnFlashTimerExpired()
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue("FlashAmount", 0.0f);
        DynamicMaterial->SetScalarParameterValue("EmissiveBoost", 1.0f);
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
    if (bFlippingLocked || Velocity.SizeSquared() < 10.f) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // rotate whole actor to movement direction
    float TargetYaw = (Velocity.Y > 0) ? 90.0f : -90.0f;
    Owner->SetActorRotation(FRotator(0, TargetYaw, 0));

    ApplyFlipping(Velocity.Y > 0 ? 1.0f : -1.0f, BaseScale);
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

    float InversionMult = bInvertVisualFlipping ? -1.0f : 1.0f;

    float FinalScaleX = DesiredWorldDirY * FMath::Abs(BaseScale) * InversionMult;

    NewScale.X = FinalScaleX;
    MeshComp->SetRelativeScale3D(NewScale);
}

void UCPP_VisualComponent::SetStatusOverlay(FLinearColor OverlayColor, float Intensity, float EmissiveBoost)
{
    if (!MeshComp) return;
    if (!DynamicMaterial) DynamicMaterial = MeshComp->CreateDynamicMaterialInstance(0);

    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue("StatusColor", OverlayColor);
        DynamicMaterial->SetScalarParameterValue("StatusIntensity", Intensity);
        DynamicMaterial->SetScalarParameterValue("EmissiveBoost", EmissiveBoost);
        UE_LOG(LogTemp, Warning, TEXT("Status Applied: %s with Intensity %f"), *OverlayColor.ToString(), Intensity);
    }
}

void UCPP_VisualComponent::ClearStatusOverlay()
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue("StatusIntensity", 0.0f);
    }
}

FVector UCPP_VisualComponent::GetVisualFacingDirection() const
{
    if (!MeshComp) return FVector(0, 1, 0);

    float CurrentScaleX = MeshComp->GetRelativeScale3D().X;
    float InversionMult = bInvertVisualFlipping ? -1.0f : 1.0f;

    // if ScaleX * Inversion > 0, facing +Y
    return (CurrentScaleX * InversionMult > 0.0f) ? FVector(0, 1, 0) : FVector(0, -1, 0);
}
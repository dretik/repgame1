// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseCharacter.h"
#include "GameplayTagsManager.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/Controller.h"
#include "PaperFlipbook.h"
#include "Engine/Engine.h"


ACPP_BaseCharacter::ACPP_BaseCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

    if (MoveComponent)
    {
        MoveComponent->GravityScale = 1.0f;
        MoveComponent->AirControl = 0.5f;
        MoveComponent->MaxWalkSpeed = 450.0f;
        MoveComponent->JumpZVelocity = 600.0f;
    }
}

void ACPP_BaseCharacter::SetupPlayerInputComponent(UInputComponent* 
    PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    //axis
    PlayerInputComponent->BindAxis("MoveRight", this, 
        &ACPP_BaseCharacter::MoveRight);
    PlayerInputComponent->BindAxis("MoveForward", this, 
        &ACPP_BaseCharacter::MoveForward);

    //actions
    PlayerInputComponent->BindAction("JumpAction", 
        IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("DodgeAction", 
        IE_Pressed, this, &ACPP_BaseCharacter::Dodge);
    PlayerInputComponent->BindAction("InteractAction",
        IE_Pressed, this, &ACPP_BaseCharacter::InteractWithObject);


    //attack
    PlayerInputComponent->BindAction("AttackAction", 
        IE_Pressed, this, &ACPP_BaseCharacter::Attack);
}

void ACPP_BaseCharacter::MoveRight(float Value)
{
    if (bIsDead) return;
    AddMovementInput(GetActorRightVector(), Value);

    if (Value > 0.0f)
    {
        GetSprite()->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
    }
    else if (Value < 0.0f)
    {
        GetSprite()->SetRelativeScale3D(FVector(-1.0f, 1.0f, 1.0f));
    }
}

void ACPP_BaseCharacter::MoveForward(float Value)
{
    if (bIsDead) return;
    AddMovementInput(GetActorForwardVector(), Value);
}

void ACPP_BaseCharacter::Dodge()
{
    if (bIsDead) return;
    if (bIsDodging) return;
    if (!CharacterStats) return;
    bIsDodging = true;
    bIsInAir = true;
    bIsInvulnerable = true;

    if (DodgeAnimationFlipbook)
    {
        GetSprite()->SetFlipbook(DodgeAnimationFlipbook);
    }

    GetSprite()->SetSpriteColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.7f));
    
    FVector RightDirection = GetActorRightVector();

    if (GetSprite()->GetRelativeScale3D().X < 0.0f)
    {
        RightDirection *= -1.0f;
    }
    const FVector DodgeHorizontalDirection = RightDirection * -1.0f;
    const FVector UpDirection = FVector::UpVector;
    FVector CombinedDirection = DodgeHorizontalDirection + UpDirection*0.5;
    CombinedDirection.Normalize();

    const FVector DodgeDirection = RightDirection * -1.0f;

    LaunchCharacter(CombinedDirection * CharacterStats->DodgeStrength, true, false);

    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this, 
        &ACPP_BaseCharacter::StopDodge, CharacterStats->DodgeDuration, false);
}

void ACPP_BaseCharacter::StopDodge()
{
    bIsDodging = false;
    bIsInvulnerable = false;

    GetSprite()->SetSpriteColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}
void ACPP_BaseCharacter::StopJump()
{
    bIsJumping = false;
}

void ACPP_BaseCharacter::Attack()
{
    if (bIsDead) return;
    if (!CharacterStats) return;
    if (bIsAttacking) return;
    ComboCounter++;
    bIsAttacking = true;

    GetWorldTimerManager().ClearTimer(ComboResetTimer);

    float CurrentAttackDuration = 0.5f;

    FVector RightDirection = GetActorRightVector();
    if (GetSprite()->GetRelativeScale3D().X < 0.0f)
    {
        RightDirection *= -1.0f;
    }

    switch ((EAttackPhase)ComboCounter)
    {
    case EAttackPhase::LightAttack:
        if (Attack1Flipbook) GetSprite()->SetFlipbook(Attack1Flipbook);

        CurrentAttackDuration = CharacterStats->LightAttackDuration;

        PerformAttackTrace(
            CharacterStats->LightAttackRange,
            CharacterStats->LightAttackBoxSize,
            CharacterStats->LightAttackDamage
        );
        break;

    case EAttackPhase::DashAttack:
        if (Attack2Flipbook) GetSprite()->SetFlipbook(Attack2Flipbook);

        LaunchCharacter(RightDirection * CharacterStats->DashAttackImpulse, true, true);
        CurrentAttackDuration = CharacterStats->DashAttackDuration;

        PerformAttackTrace(
            CharacterStats->DashAttackRange,
            CharacterStats->DashAttackBoxSize,
            CharacterStats->DashAttackDamage
        );
        break;
    case EAttackPhase::HeavyAttack:
        if (Attack3Flipbook) GetSprite()->SetFlipbook(Attack3Flipbook);

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = CharacterStats->HeavyAttackWalkSpeed;
        }
        CurrentAttackDuration = CharacterStats->HeavyAttackDuration;

        PerformAttackTrace(
            CharacterStats->HeavyAttackRange,
            CharacterStats->HeavyAttackBoxSize,
            CharacterStats->HeavyAttackDamage
        );
        break;

    default:
        ResetCombo();
        return;
    }

    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this, 
        &ACPP_BaseCharacter::AttackEnd, CurrentAttackDuration, false);
}

void ACPP_BaseCharacter::AttackEnd()
{
    bIsAttacking = false;

    if (ComboCounter == 3)
    {
        GetCharacterMovement()->MaxWalkSpeed = CharacterStats->MaxWalkSpeed;
    }

    float ResetTime = CharacterStats->ComboResetTime;

    GetWorldTimerManager().SetTimer(ComboResetTimer, this,
        &ACPP_BaseCharacter::ResetCombo, ResetTime, false);
}

void ACPP_BaseCharacter::ResetCombo()
{
    ComboCounter = 0;
    bIsAttacking = false;

    if (CharacterStats && GetCharacterMovement()) {
        GetCharacterMovement()->MaxWalkSpeed = CharacterStats->MaxWalkSpeed;
    }
}

void ACPP_BaseCharacter::PerformAttackTrace(float Range, FVector BoxSize, float DamageAmount)
{
    float DirectionSign = (GetSprite()->GetRelativeScale3D().X > 0.0f) ? 1.0f : -1.0f;

    FVector AttackDirection = FVector(0.0f, 1.0f, 0.0f) * DirectionSign;

    const FVector Start = GetActorLocation();
    const FVector End = Start + (AttackDirection * Range);
    const FVector HalfSize = BoxSize;

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    TArray<FHitResult> HitResults;

    bool bHit = UKismetSystemLibrary::BoxTraceMulti(
        GetWorld(),
        Start,
        End,
        HalfSize,
        FRotator::ZeroRotator,
        UEngineTypes::ConvertToTraceType(ECC_WorldDynamic),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::ForDuration,
        HitResults,
        true
    );

    if (bHit)
    {
        for (const FHitResult& Result : HitResults)
        {
            AActor* HitActor = Result.GetActor();

            if (HitActor && CanDealDamageTo(HitActor))
            {
                float DamageToApply = DamageAmount * CurrentDamageMultiplier;
                if ((EAttackPhase)ComboCounter == EAttackPhase::HeavyAttack)
                {
                    DamageToApply *= CharacterStats->HeavyAttackMultiplier;
                }

                UGameplayStatics::ApplyDamage(
                    HitActor,
                    DamageToApply,
                    GetController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
    }
}

void ACPP_BaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (CharacterStats)
    {

        CurrentHealth = CharacterStats->MaxHealth;

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = CharacterStats->MaxWalkSpeed;
            GetCharacterMovement()->JumpZVelocity = CharacterStats->JumpVelocity;
            GetCharacterMovement()->GravityScale = CharacterStats->GravityScale;
            GetCharacterMovement()->AirControl = CharacterStats->AirControl;
        }
    }
    else {
        CurrentHealth = 100.0f;
    }
}

void ACPP_BaseCharacter::OnJumped_Implementation()
{
    if (bIsDead) return;
    Super::OnJumped_Implementation();
    if (bIsJumping) return;
    if (!CharacterStats) return;
    bIsJumping = true;
    bIsInAir = true;

    if (JumpAnimationFlipbook)
    {
        GetSprite()->SetFlipbook(JumpAnimationFlipbook);
    }

    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this,
        &ACPP_BaseCharacter::StopJump, CharacterStats->JumpDuration, false);
}

float ACPP_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsInvulnerable)
    {
        return 0.0f;
    }
    
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth -= ActualDamage;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("%s took %f damage. Current Health: %f"), *GetName(), ActualDamage, CurrentHealth));
    }

    OnHealthChanged.Broadcast(CurrentHealth, CharacterStats->MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s has died!"), *GetName()));
        }
        OnDeath();
    }

    return ActualDamage;
}

void ACPP_BaseCharacter::OnDeath_Implementation()
{
    if (bIsDead) return;

    bIsDead = true;

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (GetController())
    {
        GetController()->StopMovement();
        GetController()->UnPossess(); 
    }

    float DeathDuration = 0.6f; 

    if (DeathAnimationFlipbook)
    {
        GetSprite()->SetFlipbook(DeathAnimationFlipbook);
        GetSprite()->SetLooping(false);
        DeathDuration = DeathAnimationFlipbook->GetTotalDuration();
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ACPP_BaseCharacter::SwitchToDeadStatic, DeathDuration, false);
}

void ACPP_BaseCharacter::SwitchToDeadStatic()
{
    if (DeadStaticFlipbook)
    {
        GetSprite()->SetFlipbook(DeadStaticFlipbook);
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ACPP_BaseCharacter::DestroyCharacter, 2.0f, false);
}

void ACPP_BaseCharacter::DestroyCharacter()
{
    Destroy();
}

void ACPP_BaseCharacter::InteractWithObject()
{
    if (bIsDead) return;

    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_Interact(Actor, this);

            return;
        }
    }
}

bool ACPP_BaseCharacter::CanDealDamageTo(AActor* TargetActor) const
{
    if (TargetActor == this) return false; 
    return true;
}

void ACPP_BaseCharacter::ApplyStatModifier(FStatModifier Modifier)
{
    float* StatToModify = nullptr;

    if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.Health"))))
    {
        StatToModify = &CurrentHealth;
    }
    else if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.Damage"))))
    {
        StatToModify = &CurrentDamageMultiplier;
    }
    else if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.Speed"))))
    {
        if (GetCharacterMovement())
        {
            StatToModify = &GetCharacterMovement()->MaxWalkSpeed;
        }
    }

    if (StatToModify)
    {
        if (Modifier.bIsMultiplier)
        {
            *StatToModify *= Modifier.Value;
        }
        else
        {
            *StatToModify += Modifier.Value;
        }

        if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.Health"))))
        {
            OnHealthChanged.Broadcast(CurrentHealth, CharacterStats ? CharacterStats->MaxHealth : 100.f);
            if (CurrentHealth <= 0) OnDeath();
        }

        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Stat Modified via Tags!"));
    }
}
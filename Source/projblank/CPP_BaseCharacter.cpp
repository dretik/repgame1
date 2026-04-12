// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_BaseCharacter.h"
#include "GameplayTagsManager.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/Controller.h"
#include "PaperFlipbook.h"
#include "CPP_Projectile.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "CPP_DamageTextActor.h"
#include "CPP_GameInstance.h"
#include "CPP_SaveGame.h"
#include "CPP_AttributeComponent.h"
#include "CPP_Action.h"   
#include "CPP_ActionComponent.h"
#include "Engine/Engine.h"

ACPP_BaseCharacter::ACPP_BaseCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

    AttributeComp = CreateDefaultSubobject<UCPP_AttributeComponent>(TEXT("AttributeComp"));

    ActionComp = CreateDefaultSubobject<UCPP_ActionComponent>(TEXT("ActionComp"));

    if (MoveComponent)
    {
        MoveComponent->GravityScale = 1.0f;
        MoveComponent->AirControl = 0.5f;
        MoveComponent->MaxWalkSpeed = 450.0f;
        MoveComponent->JumpZVelocity = 600.0f;
    }
}

//void ACPP_BaseCharacter::PerformAttackTrace(float Range, FVector BoxSize, float DamageAmount)
//{
//    if (CharacterStats && CharacterStats->AttackEffect)
//    {
//        FRotator EffectRotation = FRotator::ZeroRotator;
//
//        if (GetSprite()->GetRelativeScale3D().X > 0.0f)
//        {
//            EffectRotation.Yaw = 90.0f;
//        }
//        else {
//            EffectRotation.Yaw = -90.0f;
//        }
//
//        SpawnParticle(CharacterStats->AttackEffect, GetActorLocation(), EffectRotation);
//    }
//
//    float DirectionSign = (GetSprite()->GetRelativeScale3D().X > 0.0f) ? 1.0f : -1.0f;
//
//    FVector AttackDirection = FVector(0.0f, 1.0f, 0.0f) * DirectionSign;
//
//    const FVector Start = GetActorLocation();
//    const FVector End = Start + (AttackDirection * Range);
//    const FVector HalfSize = BoxSize;
//
//    TArray<AActor*> ActorsToIgnore;
//    ActorsToIgnore.Add(this);
//    TArray<FHitResult> HitResults;
//
//    bool bHit = UKismetSystemLibrary::BoxTraceMulti(
//        GetWorld(),
//        Start,
//        End,
//        HalfSize,
//        FRotator::ZeroRotator,
//        UEngineTypes::ConvertToTraceType(ECC_WorldDynamic),
//        false,
//        ActorsToIgnore,
//        EDrawDebugTrace::ForDuration,
//        HitResults,
//        true
//    ); 
//
//    if (bHit)
//    {
//        TSet<AActor*> DamagedActors;
//        for (const FHitResult& Result : HitResults)
//        {
//            AActor* HitActor = Result.GetActor();
//
//            if (HitActor && CanDealDamageTo(HitActor)&&!DamagedActors.Contains(HitActor))
//            {
//                DamagedActors.Add(HitActor);
//
//                UGameplayStatics::ApplyDamage(
//                    HitActor,
//                    DamageAmount,
//                    GetController(),
//                    this,
//                    UDamageType::StaticClass()
//                );
//            }
//        }
//    }
//}

void ACPP_BaseCharacter::OnSaveGame_Implementation(UCPP_SaveGame* SaveObject)
{
}

void ACPP_BaseCharacter::OnLoadGame_Implementation(UCPP_SaveGame* SaveObject)
{
}

void ACPP_BaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    //BaseSpriteScale = FMath::Abs(GetSprite()->GetRelativeScale3D().X);
    FVector StartScale = GetSprite()->GetRelativeScale3D();
    StartScale.X = BaseSpriteScale;
    StartScale.Y = BaseSpriteScale;
    StartScale.Z = BaseSpriteScale;
    GetSprite()->SetRelativeScale3D(StartScale);

    if (CharacterStats)
    {
        if (AttributeComp)
        {
            AttributeComp->InitializeStats(CharacterStats->MaxHealth);

            //subcribing on component
            AttributeComp->OnHealthChanged.AddDynamic(this, &ACPP_BaseCharacter::OnHealthChangedCallback);
            AttributeComp->OnDeath.AddDynamic(this, &ACPP_BaseCharacter::OnDeathStarted);
        }
        CurrentBaseDamage = CharacterStats->BaseDamage;

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = CharacterStats->MaxWalkSpeed;
            GetCharacterMovement()->JumpZVelocity = CharacterStats->JumpVelocity;
            GetCharacterMovement()->GravityScale = CharacterStats->GravityScale;
            GetCharacterMovement()->AirControl = CharacterStats->AirControl;
        }
    }
    else {
        if (AttributeComp)
        {
            AttributeComp->InitializeStats(100.0f);
        }
        CurrentBaseDamage = 10.0f;
    }
    if (AttributeComp)
    {
        OnHealthChanged.Broadcast(AttributeComp->GetHealth(), AttributeComp->GetMaxHealth());
    }
}

float ACPP_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsInvulnerable || !AttributeComp)
    {
        return 0.0f;
    }
    
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    bool bApplied = AttributeComp->ApplyHealthChange(DamageCauser, -ActualDamage);

    if (!bApplied && ActualDamage > 0.0f)
    {
        return 0.0f;
    }

    if (ActualDamage > 0.0f && DamageTextClass)
    {
        FVector SpawnLoc = GetActorLocation();
        SpawnLoc.Z += FMath::RandRange(50.f, 70.f);
        SpawnLoc.X += FMath::RandRange(-20.f, 20.f);
        SpawnLoc.Y += FMath::RandRange(-20.f, 20.f);

        ACPP_DamageTextActor* DmgText = GetWorld()->SpawnActor<ACPP_DamageTextActor>(
            DamageTextClass,
            SpawnLoc,
            FRotator::ZeroRotator
            );

        if (DmgText)
        {
            DmgText->UpdateDamageText(ActualDamage);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("%s took %f damage. Current Health: %f"), *GetName(), ActualDamage, AttributeComp->GetHealth()));
    }

    if (CharacterStats)
    {
        SpawnParticle(CharacterStats->HitEffect, GetActorLocation());
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

    if (CharacterStats)
    {
        SpawnParticle(CharacterStats->DeathEffect, GetActorLocation());
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

bool ACPP_BaseCharacter::CanDealDamageTo(AActor* TargetActor) const
{
    if (TargetActor == this) return false; 
    return true;
}

void ACPP_BaseCharacter::ApplyStatModifier(FStatModifier Modifier)
{
    //TMap vocab
    //TMap<FGameplayTag, float*> StatMap;
    //StatMap.Add(FGameplayTag::RequestGameplayTag("Stats.Damage"), &DamageMultiplier);
    //StatMap.Add(FGameplayTag::RequestGameplayTag("Stats.Speed"), &MaxWalkSpeed);
    
    if (GEngine)
    {
        FString IncomingTagName = Modifier.StatTag.ToString();
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
            FString::Printf(TEXT("Incoming Tag: '%s'"), *IncomingTagName));
    }

    float* StatToModify = nullptr;

    const FGameplayTag Tag_Health = FGameplayTag::RequestGameplayTag(FName("Stats.Health"));
    const FGameplayTag Tag_HealthMax = FGameplayTag::RequestGameplayTag(FName("Stats.HealthMax"));

    if (Modifier.StatTag.MatchesTagExact(Tag_Health))
    {
        if (AttributeComp)
        {
            float Delta = Modifier.Value;
            if (Modifier.bIsMultiplier)
            {
                float Current = AttributeComp->GetHealth();
                Delta = (Current * Modifier.Value) - Current;
            }

            AttributeComp->ApplyHealthChange(this, Delta);

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
                FString::Printf(TEXT("Health Modified by: %f"), Delta));
        }
        return;
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_HealthMax))
    {
        if (AttributeComp)
        {
            float NewMax = AttributeComp->GetMaxHealth();
            if (Modifier.bIsMultiplier)
            {
                NewMax *= Modifier.Value;
            }
            else
            {
                NewMax += Modifier.Value;
            }

            //default logic -> heal to maxhp
            //to do without healing - must add SetMaxHealth method in component
            AttributeComp->InitializeStats(NewMax);

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
                FString::Printf(TEXT("Max Health Updated: %f"), NewMax));
        }
        return;
    }

    const FGameplayTag Tag_Damage = FGameplayTag::RequestGameplayTag(FName("Stats.Damage"));
    const FGameplayTag Tag_Speed = FGameplayTag::RequestGameplayTag(FName("Stats.Speed"));

    if (Modifier.StatTag.MatchesTagExact(Tag_Damage))
    {
        if (Modifier.bIsMultiplier)
        {
            if (AttributeComp)
            {
                float OldMult = AttributeComp->GetDamageMultiplier();
                float NewMult = OldMult * Modifier.Value;
                AttributeComp->SetDamageMultiplier(NewMult);

                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Damage Multiplier Updated: %f"), NewMult));
            }
        }
        else
        {
            CurrentBaseDamage += Modifier.Value;
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Base Damage Updated: %f"), CurrentBaseDamage));
        }
        return;
    }

    if (Modifier.StatTag.MatchesTagExact(Tag_Speed))
    {
        if (GetCharacterMovement())
        {
            if (Modifier.bIsMultiplier)
            {
                GetCharacterMovement()->MaxWalkSpeed *= Modifier.Value;
            }
            else
            {
                GetCharacterMovement()->MaxWalkSpeed += Modifier.Value;
            }

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Speed Updated: %f"), GetCharacterMovement()->MaxWalkSpeed));
        }
    }
}

void ACPP_BaseCharacter::SpawnParticle(UNiagaraSystem* Effect, FVector Location, FRotator Rotation)
{
    if (Effect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Effect,
            Location,
            Rotation,
            FVector(1.f), // Scale
            true, // Auto Destroy
            true, // Auto Activate
            ENCPoolMethod::None,
            true // PreCullCheck
        );
    }
}

void ACPP_BaseCharacter::OnHealthChangedCallback(AActor* InstigatorActor, UCPP_AttributeComponent* OwningComp, float NewHealth, float Delta)
{
    if (Delta < 0.0f && CharacterStats && CharacterStats->HitEffect)
    {
        SpawnParticle(CharacterStats->HitEffect, GetActorLocation());
    }
    if (OwningComp)
    {
        OnHealthChanged.Broadcast(NewHealth, OwningComp->GetMaxHealth());
    }
}

void ACPP_BaseCharacter::OnDeathStarted(AActor* Killer)
{
    OnDeath();
    //if actor is static (located in editor) he does not have dynamic tag
    if (!Tags.Contains(FName("Dynamic")))
    {
        UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
        if (GI)
        {
            GI->DestroyedStaticActors.Add(GetName());
        }
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s died!"), *GetName()));
    }
}

float ACPP_BaseCharacter::GetCurrentHealth() const
{
    return AttributeComp ? AttributeComp->GetHealth() : 0.0f;
}

float ACPP_BaseCharacter::GetCurrentMaxHealth() const
{
    return AttributeComp ? AttributeComp->GetMaxHealth() : 0.0f;
}

bool ACPP_BaseCharacter::GetIsAttacking() const
{
    if (ActionComp)
    {
        return ActionComp->ActiveGameplayTags.HasTag(FGameplayTag::RequestGameplayTag("Ability"));
    }
    return false;
}

void ACPP_BaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead || bIsDodging || GetIsAttacking())
    {
        return;
    }

    if (GetCharacterMovement())
    {
        float VelocityY = GetVelocity().Y;

        const float FlipThreshold = 0.1f;

        FVector NewScale = GetSprite()->GetRelativeScale3D();

        if (VelocityY > FlipThreshold)
        {
            NewScale.X = BaseSpriteScale;
        }
        else if (VelocityY < -FlipThreshold)
        {
            NewScale.X = -BaseSpriteScale;
        }
        GetSprite()->SetRelativeScale3D(NewScale);
    }
}
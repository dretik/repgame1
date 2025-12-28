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
    PlayerInputComponent->BindAction("CastAction",
        IE_Pressed, this, &ACPP_BaseCharacter::CastFireball);
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
            CurrentBaseDamage
        );
        break;

    case EAttackPhase::DashAttack:
        if (Attack2Flipbook) GetSprite()->SetFlipbook(Attack2Flipbook);

        LaunchCharacter(RightDirection * CharacterStats->DashAttackImpulse, true, true);
        CurrentAttackDuration = CharacterStats->DashAttackDuration;

        PerformAttackTrace(
            CharacterStats->DashAttackRange,
            CharacterStats->DashAttackBoxSize,
            CurrentBaseDamage
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
            CurrentBaseDamage
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
    if (CharacterStats && CharacterStats->AttackEffect)
    {
        FRotator EffectRotation = FRotator::ZeroRotator;

        if (GetSprite()->GetRelativeScale3D().X > 0.0f)
        {
            EffectRotation.Yaw = 90.0f;
        }
        else {
            EffectRotation.Yaw = -90.0f;
        }

        SpawnParticle(CharacterStats->AttackEffect, GetActorLocation(), EffectRotation);
    }

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

        CurrentMaxHealth = CharacterStats->MaxHealth;
        CurrentHealth = CurrentMaxHealth;
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
        CurrentMaxHealth = 100.0f;
        CurrentHealth = 100.0f;
        CurrentBaseDamage = 10.0f;
    }
    OnHealthChanged.Broadcast(CurrentHealth, CurrentMaxHealth);
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

    if (CurrentHealth < 0.0f)
    {
        CurrentHealth = 0.0f;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("%s took %f damage. Current Health: %f"), *GetName(), ActualDamage, CurrentHealth));
    }

    if (CharacterStats)
    {
        SpawnParticle(CharacterStats->HitEffect, GetActorLocation());
    }

    OnHealthChanged.Broadcast(CurrentHealth, CurrentMaxHealth);

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
    const FGameplayTag Tag_Damage = FGameplayTag::RequestGameplayTag(FName("Stats.Damage"));
    const FGameplayTag Tag_Speed = FGameplayTag::RequestGameplayTag(FName("Stats.Speed"));
    const FGameplayTag Tag_CoinMult = FGameplayTag::RequestGameplayTag(FName("Stats.CoinMultiplier"));

    if (Modifier.StatTag.MatchesTagExact(Tag_Health))
    {
        StatToModify = &CurrentHealth;
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_HealthMax))
    {
        StatToModify = &CurrentMaxHealth;
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_Damage))
    {
        if (Modifier.bIsMultiplier)
        {
            StatToModify = &CurrentDamageMultiplier;
        }
        else
        {
            StatToModify = &CurrentBaseDamage;
        }
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_Speed))
    {
        if (GetCharacterMovement())
        {
            StatToModify = &GetCharacterMovement()->MaxWalkSpeed;
        }
    }
    //else if (Modifier.StatTag.MatchesTagExact(Tag_CoinMult))
    //{
        //if (Modifier.bIsMultiplier)
        //{
            //StatToModify = &CoinMultiplier;
        //}
        //else
        //{
        //    StatToModify = &CoinCount;
        //}
        
    //}


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
            if (*StatToModify > CurrentMaxHealth) *StatToModify = CurrentMaxHealth;
            if (*StatToModify < 0) *StatToModify = 0;
            OnHealthChanged.Broadcast(CurrentHealth,CurrentMaxHealth);
            if (CurrentHealth <= 0) OnDeath();
        }
        else if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.HealthMax"))))
        {
            if (!Modifier.bIsMultiplier && Modifier.Value > 0.0f)
            {
                CurrentHealth += Modifier.Value;
            }

            OnHealthChanged.Broadcast(CurrentHealth, CurrentMaxHealth);
        }
        else if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.Damage"))))
        {
            if (Modifier.bIsMultiplier)
            {
                CurrentDamageMultiplier *= Modifier.Value;

                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
                    FString::Printf(TEXT("Damage Multiplier Increased: %f"), CurrentDamageMultiplier));
            }
            else
            {
                CurrentBaseDamage += Modifier.Value;

                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
                    FString::Printf(TEXT("Base Damage Increased: %f"), CurrentBaseDamage));
            }
        }  
        else if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.CoinMultiplier"))))
        {
            if (Modifier.bIsMultiplier)
            {
                CoinMultiplier *= Modifier.Value;
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
                    FString::Printf(TEXT("Coin Multiplier is now: %f"), CoinMultiplier));
            }
            else
            {
                CoinMultiplier += Modifier.Value;
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
                    FString::Printf(TEXT("Coin Multiplier is now: %f"), CoinMultiplier));
            }
            return;
        }
        else if (Modifier.StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Stats.Gold"))))
        {

            if (Modifier.bIsMultiplier)
            {
                int32 CurrentGold = CoinCount;
                int32 BonusGold = FMath::FloorToInt(CurrentGold * (Modifier.Value - 1.0f));
                AddCoins(BonusGold);
            }
            else
            {
                int32 CoinsToAdd = FMath::RoundToInt(Modifier.Value);
                AddCoins(CoinsToAdd);
            }

            return;
        }

        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
                FString::Printf(TEXT("Stat Changed! New Value: %f"), *StatToModify));
    }
}

int32 ACPP_BaseCharacter::GrantAbility(FGameplayTag AbilityTag, int32 MaxLevel)
{
    if (!AbilityTag.IsValid()) return 0;

    if (AbilityLevels.Contains(AbilityTag))
    {
        int32 CurrentLevel = AbilityLevels[AbilityTag];

        if (CurrentLevel < MaxLevel)
        {
            AbilityLevels[AbilityTag] = CurrentLevel + 1;
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
                FString::Printf(TEXT("Ability UPGRADED to Level %d!"), CurrentLevel + 1));
        }
        else
        {
            // if ability already max leveled
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Ability Already Max Level!"));
        }
    }
    else
    {
        AbilityLevels.Add(AbilityTag, 1);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("New Ability Unlocked!"));
    }

    return AbilityLevels[AbilityTag];
}

int32 ACPP_BaseCharacter::GetAbilityLevel(FGameplayTag AbilityTag) const
{
    if (AbilityLevels.Contains(AbilityTag))
    {
        return AbilityLevels[AbilityTag];
    }
    return 0;
}

bool ACPP_BaseCharacter::HasAbility(FGameplayTag AbilityTag) const
{
    return GetAbilityLevel(AbilityTag) > 0;
}

void ACPP_BaseCharacter::CastFireball()
{
    if (bIsDead || bIsAttacking) return;

    if (!bCanCastSpell)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Spell on Cooldown!"));
        return;
    }

    if (!HasAbility(FGameplayTag::RequestGameplayTag("Ability.Fireball")))
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Ability Locked!"));
        return;
    }

    int32 Level = GetAbilityLevel(FGameplayTag::RequestGameplayTag("Ability.Fireball"));
    if (Level <= 0) return;

    bCanCastSpell = false;
    GetWorldTimerManager().SetTimer(SpellCooldownTimer, this, &ACPP_BaseCharacter::ResetSpellCooldown, FireballCooldown, false);

    if (CharacterStats && CharacterStats->AttackEffect)
    {
        FRotator EffectRotation = FRotator::ZeroRotator;

        if (GetSprite()->GetRelativeScale3D().X > 0.0f)
        {
            EffectRotation.Yaw = 90.0f;
        }
        else {
            EffectRotation.Yaw = -90.0f;
        }

        SpawnParticle(CharacterStats->AttackEffect, GetActorLocation(), EffectRotation);
    }
    
    int32 Index = FMath::Clamp(Level - 1, 0, FireballLevels.Num() - 1);

    //cast flipbook to be added
    if (FireballLevels.Num() > 0)
    {
        TSubclassOf<ACPP_Projectile> ProjectileClass = FireballLevels[Index];

        if (ProjectileClass)
        {
            FVector SpawnLocation = GetActorLocation();
            SpawnLocation.Z += 20.0f;
            FRotator SpawnRotation = FRotator::ZeroRotator;

            if (GetSprite()->GetRelativeScale3D().X > 0.0f)
            {
                SpawnRotation.Yaw = 90.0f;
                SpawnLocation.Y += 40.0f;
            }
            else
            {
                SpawnRotation.Yaw = -90.0f;
                SpawnLocation.Y -= 40.0f;
            }

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();

            GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
        }
    }
}

void ACPP_BaseCharacter::ResetSpellCooldown()
{
    bCanCastSpell = true;
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Spell Ready!"));
}

void ACPP_BaseCharacter::ShowNotification(FText Text, FLinearColor Color)
{
    OnNotificationReceived.Broadcast(Text, Color);
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

void ACPP_BaseCharacter::AddCoins(int32 Amount)
{
    int32 FinalAmount = FMath::FloorToInt(Amount * CoinMultiplier);

    CoinCount += FinalAmount;

    OnCoinsUpdated.Broadcast(CoinCount);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Coins: %d (+%d)"), CoinCount, FinalAmount));
}
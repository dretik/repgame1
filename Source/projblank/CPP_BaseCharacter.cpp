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
        IE_Pressed, this, &ACPP_BaseCharacter::PrimaryAttack);
    PlayerInputComponent->BindAction("CastAction",
        IE_Pressed, this, &ACPP_BaseCharacter::MagicAttack);
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

void ACPP_BaseCharacter::ExecuteMeleeAttack()
{
    if (!ActionComp) return;
    if (bIsDead) return;

    FGameplayTag SelectedTag;

    // Выбор тега (как у тебя было)
    switch (ComboCounter)
    {
    case 0: SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.LightAttack"); break;
    case 1: SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.DashAttack"); break;
    case 2: SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.HeavyAttack"); break;
    default: SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.LightAttack"); break;
    }

    // Запускаем действие
    // ВНИМАНИЕ: Мы НЕ вызываем StopActionByName. Пусть старое действие закончится само!
    bool bStarted = ActionComp->StartActionByName(this, SelectedTag);

    if (bStarted)
    {
        // Сброс буфера, так как мы только что исполнили желание игрока
        bInputBuffered = false;

        // ИЩЕМ ЗАПУЩЕННЫЙ ЭКШЕН, ЧТОБЫ ПОДПИСАТЬСЯ
        // (Нам нужно получить указатель на сам объект Action)
        UCPP_Action* ActiveAction = ActionComp->GetAction(SelectedTag); // <-- Эту функцию надо добавить в ActionComp (см. ниже)

        if (ActiveAction)
        {
            // Подписываемся на его завершение
            ActiveAction->OnActionStopped.AddDynamic(this, &ACPP_BaseCharacter::OnAttackActionStopped);
        }

        // Таймер сброса комбо очищаем, пока мы в бою
        GetWorldTimerManager().ClearTimer(ComboResetTimer);
    }
}
void ACPP_BaseCharacter::OnAttackActionStopped(UCPP_Action* Action)
{
    // Отписываемся, чтобы не вызывать это повторно для того же экшена
    if (Action)
    {
        Action->OnActionStopped.RemoveDynamic(this, &ACPP_BaseCharacter::OnAttackActionStopped);
    }

    // Если игрок нажал кнопку во время удара (в буфере есть заказ)
    if (bInputBuffered)
    {
        // Переходим к следующему шагу
        ComboCounter++;
        if (ComboCounter > 2) ComboCounter = 0; // Цикл или сброс

        ExecuteMeleeAttack();
    }
    else
    {
        // Если игрок ничего не нажимал - запускаем таймер сброса комбо
        // (чтобы если он нажмет через 0.2 сек, комбо продолжилось, а не сбросилось сразу)
        float ResetTime = (CharacterStats) ? CharacterStats->ComboResetTime : 1.0f;
        GetWorldTimerManager().SetTimer(ComboResetTimer, this, &ACPP_BaseCharacter::ResetCombo, ResetTime, false);
    }
}
void ACPP_BaseCharacter::Attack()
{
    if (bIsDead) return;
    if (GetIsAttacking())
    {
        bInputBuffered = true;
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Input Buffered!"));
    }
    else {
        ComboCounter = 0;
        ExecuteMeleeAttack();
    }

    FVector RightDirection = GetActorRightVector();
    if (GetSprite()->GetRelativeScale3D().X < 0.0f)
    {
        RightDirection *= -1.0f;
    }
}

void ACPP_BaseCharacter::ResetCombo()
{
    ComboCounter = 0;

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
        TSet<AActor*> DamagedActors;
        for (const FHitResult& Result : HitResults)
        {
            AActor* HitActor = Result.GetActor();

            if (HitActor && CanDealDamageTo(HitActor)&&!DamagedActors.Contains(HitActor))
            {
                DamagedActors.Add(HitActor);
                float DamageToApply = DamageAmount * CurrentDamageMultiplier;

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
        if (AttributeComp)
        {
            AttributeComp->InitializeStats(CharacterStats->MaxHealth);

            // ВАЖНО: Подписываемся на события компонента здесь
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

    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());

    if (GI)
    {
        if (GI && GI->bIsLoadingSave && IsPlayerControlled())
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Loading Save Data..."));

            GI->LoadGame(this);

            GI->RespawnDynamicEnemies(GetWorld());
        }
        else
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("New Game Started (Default Stats)"));
        }
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

    if (Modifier.StatTag.MatchesTagExact(Tag_Health))
    {
        if (AttributeComp)
        {
            // Если множитель (например, зелье +50% от текущего)
            float Delta = Modifier.Value;
            if (Modifier.bIsMultiplier)
            {
                // Вычисляем сколько добавить: (Health * 1.5) - Health
                float Current = AttributeComp->GetHealth();
                Delta = (Current * Modifier.Value) - Current;
            }

            AttributeComp->ApplyHealthChange(this, Delta);

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
                FString::Printf(TEXT("Health Modified by: %f"), Delta));
        }
        return; // Выходим, так как обработали
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

            // Здесь мы используем InitializeStats, что также вылечит персонажа до полного.
            // Для "Рогалика" получение MaxHP предмета часто лечит, так что это ок.
            // Если нужно только увеличить потолок, не леча, нужно добавить метод SetMaxHealth в компонент.
            AttributeComp->InitializeStats(NewMax);

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
                FString::Printf(TEXT("Max Health Updated: %f"), NewMax));
        }
        return;
    }

    const FGameplayTag Tag_Damage = FGameplayTag::RequestGameplayTag(FName("Stats.Damage"));
    const FGameplayTag Tag_Speed = FGameplayTag::RequestGameplayTag(FName("Stats.Speed"));
    const FGameplayTag Tag_CoinMult = FGameplayTag::RequestGameplayTag(FName("Stats.CoinMultiplier"));
    const FGameplayTag Tag_Gold = FGameplayTag::RequestGameplayTag(FName("Stats.Gold"));

    if (Modifier.StatTag.MatchesTagExact(Tag_Damage))
    {
        StatToModify = Modifier.bIsMultiplier ? &CurrentDamageMultiplier : &CurrentBaseDamage;
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_Speed))
    {
        if (GetCharacterMovement())
        {
            StatToModify = &GetCharacterMovement()->MaxWalkSpeed;
        }
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_CoinMult))
    {
        StatToModify = &CoinMultiplier; // Исправлено: CoinMultiplier всегда меняется здесь
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_Gold))
    {
        // Логика золота отдельная (метод AddCoins), указатель не нужен
        if (Modifier.bIsMultiplier)
        {
            int32 BonusGold = FMath::FloorToInt(CoinCount * (Modifier.Value - 1.0f));
            AddCoins(BonusGold);
        }
        else
        {
            AddCoins(FMath::RoundToInt(Modifier.Value));
        }
        return;
    }

    // Применяем изменения к float* если он найден
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

        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
                FString::Printf(TEXT("Stat Changed! New Value: %f"), *StatToModify));
    }
}

bool ACPP_BaseCharacter::GrantAbility(TSubclassOf<UCPP_Action> ActionClass)
{
    if (!ActionClass || !ActionComp) return false;

    // 1. Получаем Default Object, чтобы узнать тег способности, не создавая её
    UCPP_Action* DefaultAction = ActionClass->GetDefaultObject<UCPP_Action>();
    if (!DefaultAction) return false;

    FGameplayTag ActionTag = DefaultAction->ActionTag;

    // 2. Проверяем текущий уровень
    int32 CurrentLevel = 0;
    if (AbilityLevels.Contains(ActionTag))
    {
        CurrentLevel = AbilityLevels[ActionTag];
    }

    // 3. ЛОГИКА: Разблокировка или Апгрейд
    if (CurrentLevel == 0)
    {
        // --- РАЗБЛОКИРОВКА (Первый подбор) ---

        // Добавляем саму логику в компонент (чтобы можно было нажимать кнопку)
        ActionComp->AddAction(ActionClass);

        // Ставим уровень 1
        AbilityLevels.Add(ActionTag, 1);

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            FString::Printf(TEXT("Ability Unlocked: %s (Level 1)"), *ActionTag.ToString()));

        ShowNotification(NSLOCTEXT("Abilities", "Unlock", "New Ability Unlocked!"), FColor::Purple);
    }
    else
    {
        // --- АПГРЕЙД (Повторный подбор) ---

        // Просто повышаем уровень. ActionComponent трогать не надо, действие там уже есть.
        AbilityLevels[ActionTag] = CurrentLevel + 1;

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            FString::Printf(TEXT("Ability Upgraded: %s (Level %d)"), *ActionTag.ToString(), CurrentLevel + 1));

        ShowNotification(NSLOCTEXT("Abilities", "Upgrade", "Ability Level Up!"), FColor::Cyan);
    }

    return true;
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

//void ACPP_BaseCharacter::CastFireball()
//{
//    if (bIsDead || bIsAttacking) return;
//
//    if (!bCanCastSpell)
//    {
//        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Spell on Cooldown!"));
//        return;
//    }
//
//    if (!HasAbility(FGameplayTag::RequestGameplayTag("Ability.Fireball")))
//    {
//        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Ability Locked!"));
//        return;
//    }
//
//    int32 Level = GetAbilityLevel(FGameplayTag::RequestGameplayTag("Ability.Fireball"));
//    if (Level <= 0) return;
//
//    bCanCastSpell = false;
//    GetWorldTimerManager().SetTimer(SpellCooldownTimer, this, &ACPP_BaseCharacter::ResetSpellCooldown, FireballCooldown, false);
//
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
//    int32 Index = FMath::Clamp(Level - 1, 0, FireballLevels.Num() - 1);
//
//    //cast flipbook to be added
//    if (FireballLevels.Num() > 0)
//    {
//        TSubclassOf<ACPP_Projectile> ProjectileClass = FireballLevels[Index];
//
//        if (ProjectileClass)
//        {
//            FVector SpawnLocation = GetActorLocation();
//            SpawnLocation.Z += 20.0f;
//            FRotator SpawnRotation = FRotator::ZeroRotator;
//
//            if (GetSprite()->GetRelativeScale3D().X > 0.0f)
//            {
//                SpawnRotation.Yaw = 90.0f;
//                SpawnLocation.Y += 40.0f;
//            }
//            else
//            {
//                SpawnRotation.Yaw = -90.0f;
//                SpawnLocation.Y -= 40.0f;
//            }
//
//            FActorSpawnParameters SpawnParams;
//            SpawnParams.Owner = this;
//            SpawnParams.Instigator = GetInstigator();
//
//            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
//
//            ACPP_Projectile* Fireball = Cast<ACPP_Projectile>(SpawnedActor);
//            if (Fireball)
//            {
//                float MagicDamage = CurrentBaseDamage * CurrentDamageMultiplier;
//
//                Fireball->SetDamage(MagicDamage);
//            }
//        }
//    }
//}

//void ACPP_BaseCharacter::ResetSpellCooldown()
//{
//    bCanCastSpell = true;
//    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Spell Ready!"));
//}

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

void ACPP_BaseCharacter::AddExperience(float Amount)
{
    CurrentXP += Amount;

    while (CurrentXP >= XPToNextLevel)
    {
        CurrentXP -= XPToNextLevel;
        LevelUp();
    }

    OnXPUpdated.Broadcast(CurrentXP, XPToNextLevel, CharacterLevel);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, FString::Printf(TEXT("XP: %.0f / %.0f"), CurrentXP, XPToNextLevel));
}

void ACPP_BaseCharacter::LevelUp()
{
    CharacterLevel++;

    XPToNextLevel *= LevelUpMultiplier;

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("LEVEL UP! Level: %d"), CharacterLevel));

    float HPBonus = CharacterStats->HealthGrowthPerLevel; // Дефолт, если нет статов
    float DmgBonus = CharacterStats->DamageGrowthPerLevel;

    if (CharacterStats)
    {
        HPBonus = CharacterStats->HealthGrowthPerLevel;
        DmgBonus = CharacterStats->DamageGrowthPerLevel;

        CurrentBaseDamage += DmgBonus;

        // --- МОДУЛЬНОЕ ОБНОВЛЕНИЕ ЗДОРОВЬЯ ---
        if (AttributeComp)
        {
            float NewMaxHP = AttributeComp->GetMaxHealth() + HPBonus;

            // InitializeStats устанавливает MaxHealth и лечит персонажа до максимума
            AttributeComp->InitializeStats(NewMaxHP);
        }

        FText LevelUpMsg = FText::Format(
            NSLOCTEXT("HUD", "LevelUpDetail", "Level Up! HP +{0}, Dmg +{1}"),
            FText::AsNumber((int32)HPBonus),
            FText::AsNumber((int32)DmgBonus)
        );
        ShowNotification(LevelUpMsg, FColor::Yellow);
    }

    // Broadcast вызывается внутри InitializeStats -> OnHealthChangedCallback, 
    // поэтому вручную вызывать OnHealthChanged.Broadcast не нужно.

    // Звук или эффект можно добавить здесь
}

void ACPP_BaseCharacter::RemoveExperience(float Amount)
{
    CurrentXP -= Amount;

    if (CurrentXP < 0.0f) CurrentXP = 0.0f;

    OnXPUpdated.Broadcast(CurrentXP, XPToNextLevel, CharacterLevel);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, FString::Printf(TEXT("XP Spent: -%.0f"), Amount));
}

void ACPP_BaseCharacter::SetStatsFromSave(float SavedHealth, float SavedMaxHealth, float SavedBaseDamage, int32 SavedLevel, float SavedXP, int32 SavedCoins)
{
    if (AttributeComp)
    {
        // Нам, возможно, понадобится функция ForceSetHealth в компоненте, 
        // или просто InitializeStats(SavedMaxHealth) + ApplyHealthChange для подгонки.
        // Но лучше всего добавить в Component метод SetHealthDirectly для загрузки.

        // Пока используем InitializeStats для макс хп:
        AttributeComp->InitializeStats(SavedMaxHealth);

        // И хак через урон/лечение для текущего, чтобы выставить нужное значение:
        float Diff = SavedHealth - AttributeComp->GetHealth();
        AttributeComp->ApplyHealthChange(nullptr, Diff);
    }
    CurrentBaseDamage = SavedBaseDamage;
    CharacterLevel = SavedLevel;
    CurrentXP = SavedXP;
    CoinCount = SavedCoins;

    //OnHealthChanged.Broadcast(CurrentHealth, CurrentMaxHealth);
    OnXPUpdated.Broadcast(CurrentXP, XPToNextLevel, CharacterLevel);
    OnCoinsUpdated.Broadcast(CoinCount);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Stats Restored from Save!"));
}

void ACPP_BaseCharacter::SetAbilityLevels(const TMap<FGameplayTag, int32>& LoadedAbilities)
{
    AbilityLevels = LoadedAbilities;
}

void ACPP_BaseCharacter::SetLocationFromSave(FVector SavedLocation)
{
    SetActorLocation(SavedLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ACPP_BaseCharacter::SetCurrentHealth(float NewHealth)
{
    if (AttributeComp)
    {
        float Diff = NewHealth - AttributeComp->GetHealth();
        AttributeComp->ApplyHealthChange(nullptr, Diff);
    }

}

void ACPP_BaseCharacter::OnHealthChangedCallback(AActor* InstigatorActor, UCPP_AttributeComponent* OwningComp, float NewHealth, float Delta)
{
    // Сюда можно перенести визуальные эффекты (HitEffect)
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
    // Вызываем твою существующую логику смерти
    OnDeath();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s died!"), *GetName()));
    }
}

float ACPP_BaseCharacter::GetCurrentHealth() const
{
    // Безопасная проверка: если компонента нет, возвращаем 0
    return AttributeComp ? AttributeComp->GetHealth() : 0.0f;
}

float ACPP_BaseCharacter::GetCurrentMaxHealth() const
{
    return AttributeComp ? AttributeComp->GetMaxHealth() : 0.0f;
}

void ACPP_BaseCharacter::PrimaryAttack()
{
    // Здесь пока можно оставить старую логику Attack(), 
    // НО для диплома лучше со временем перенести и удар мечом в Action (класс CPP_Action_Melee).
    // Пока оставим как есть, чтобы не сломать всё сразу:
    Attack();
}

void ACPP_BaseCharacter::MagicAttack()
{
    if (ActionComp)
    {
        // Мы просим компонент: "Запусти действие с тегом Fireball".
        // Компонент сам проверит кулдаун, наличие маны (если добавим), состояние оглушения и т.д.
        ActionComp->StartActionByName(this, FGameplayTag::RequestGameplayTag("Ability.Player.Range.Fireball"));
    }
}

bool ACPP_BaseCharacter::GetIsAttacking() const
{
    if (ActionComp)
    {
        // Проверяем наличие любого тега категории Ability
        return ActionComp->ActiveGameplayTags.HasTag(FGameplayTag::RequestGameplayTag("Ability"));
    }
    return false;
}
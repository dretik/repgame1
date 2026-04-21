// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_PlayerCharacter.h"
#include "CPP_GameInstance.h"
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
#include "CPP_SaveGame.h"
#include "CPP_InventoryComponent.h"
#include "CPP_Action.h"
#include "CPP_ActionSet.h"
#include "CPP_ActionComponent.h"
#include "CPP_VisualComponent.h"
#include "CPP_ProgressionStatics.h"
#include "CharacterStats.h"

ACPP_PlayerCharacter::ACPP_PlayerCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ComboCounter = 0;
    bInputBuffered = false;
}

void ACPP_PlayerCharacter::SetupPlayerInputComponent(UInputComponent*
    PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    //axis
    PlayerInputComponent->BindAxis("MoveRight", this,
        &ACPP_PlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis("MoveForward", this,
        &ACPP_PlayerCharacter::MoveForward);

    //actions
    PlayerInputComponent->BindAction("JumpAction",
        IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("DodgeAction",
        IE_Pressed, this, &ACPP_PlayerCharacter::Dodge);
    PlayerInputComponent->BindAction("InteractAction",
        IE_Pressed, this, &ACPP_PlayerCharacter::InteractWithObject);
    PlayerInputComponent->BindAction("AbilityQ", 
        IE_Pressed, this, &ACPP_PlayerCharacter::InputAbilityQ);
    PlayerInputComponent->BindAction("AbilityE", 
        IE_Pressed, this, &ACPP_PlayerCharacter::InputAbilityE);
    PlayerInputComponent->BindAction("AbilityR", 
        IE_Pressed, this, &ACPP_PlayerCharacter::InputAbilityR);

    //attack
    PlayerInputComponent->BindAction("AttackAction",
        IE_Pressed, this, &ACPP_PlayerCharacter::PrimaryAttack);
    PlayerInputComponent->BindAction("CastAction",
        IE_Pressed, this, &ACPP_PlayerCharacter::MagicAttack);
}

void ACPP_PlayerCharacter::Dodge()
{
    if (bIsDead || bIsDodging || !CharacterStats || !VisualComp) return;
    bIsDodging = true;
    bIsInAir = true;
    bIsInvulnerable = true;

    VisualComp->LockFlipping(CharacterStats->DodgeDuration);

    if (DodgeAnimationFlipbook)
    {
        GetSprite()->SetFlipbook(DodgeAnimationFlipbook);
    }

    GetSprite()->SetSpriteColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.7f));

    FVector DodgeDirection = GetActorForwardVector()*-1.0f;
    DodgeDirection.Z = 0.4f;
    DodgeDirection.Normalize();

    LaunchCharacter(DodgeDirection * CharacterStats->DodgeStrength, true, true);
    
    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this,
        &ACPP_PlayerCharacter::StopDodge, CharacterStats->DodgeDuration, false);
}

void ACPP_PlayerCharacter::OnJumped_Implementation()
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
        &ACPP_PlayerCharacter::StopJump, CharacterStats->JumpDuration, false);
}

void ACPP_PlayerCharacter::InteractWithObject()
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

void ACPP_PlayerCharacter::ShowNotification(FText Text, FLinearColor Color)
{
    OnNotificationReceived.Broadcast(Text, Color);
}

bool ACPP_PlayerCharacter::GrantAbility(TSubclassOf<UCPP_Action> ActionClass)
{
    if (!ActionComp) return false;

    EActionGrantResult Result = ActionComp->GrantAction(ActionClass);

    UCPP_Action* DefaultAction = ActionClass->GetDefaultObject<UCPP_Action>();
    FGameplayTag ActionTag = DefaultAction->ActionTag;

    switch (Result)
    {
    case EActionGrantResult::Unlocked:
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            FString::Printf(TEXT("Ability Unlocked: %s (Level 1)"), *ActionTag.ToString()));

        ShowNotification(NSLOCTEXT("Abilities", "Unlock", "New Ability Unlocked!"), FColor::Purple);
        return true;
    }
    case EActionGrantResult::Upgraded:
    {
        int32 NewLevel = ActionComp->GetActionLevel(ActionTag);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            FString::Printf(TEXT("Ability Upgraded: %s (Level %d)"), *ActionTag.ToString(), NewLevel));

        ShowNotification(NSLOCTEXT("Abilities", "Upgrade", "Ability Level Up!"), FColor::Cyan);
        return true;
    }
    case EActionGrantResult::MaxLevelReached:
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Ability is already at Max Level!"));
        return false;
    }
    default:
        return false;
    }
}

//int32 ACPP_PlayerCharacter::GetAbilityLevel(FGameplayTag AbilityTag) const
//{
//    if (AbilityLevels.Contains(AbilityTag))
//    {
//        return AbilityLevels[AbilityTag];
//    }
//    return 0;
//}

//bool ACPP_PlayerCharacter::HasAbility(FGameplayTag AbilityTag) const
//{
//    return GetAbilityLevel(AbilityTag) > 0;
//}

//void ACPP_PlayerCharacter::SetAbilityLevels(const TMap<FGameplayTag, int32>& LoadedAbilities)
//{
//    AbilityLevels = LoadedAbilities;
//}

void ACPP_PlayerCharacter::AddCoins(int32 Amount)
{
    int32 FinalAmount = FMath::FloorToInt(Amount * CoinMultiplier);

    CoinCount += FinalAmount;

    OnCoinsUpdated.Broadcast(CoinCount);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("Coins: %d (+%d)"), CoinCount, FinalAmount));
}

void ACPP_PlayerCharacter::AddExperience(float Amount)
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

void ACPP_PlayerCharacter::RemoveExperience(float Amount)
{
    CurrentXP -= Amount;

    if (CurrentXP < 0.0f) CurrentXP = 0.0f;

    OnXPUpdated.Broadcast(CurrentXP, XPToNextLevel, CharacterLevel);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, FString::Printf(TEXT("XP Spent: -%.0f"), Amount));
}

void ACPP_PlayerCharacter::BeginPlay()
{
    Super::BeginPlay(); // will call AttributeComp from BaseCharacter

    UCPP_GameInstance* GI = Cast<UCPP_GameInstance>(GetGameInstance());
    if (GI)
    {
        if (GI->bIsLoadingSave && IsPlayerControlled())
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Loading Save Data..."));
            GI->LoadGame();
        }
        else
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("New Game Started"));
        }
    }
}

void ACPP_PlayerCharacter::ApplyStatModifier(FStatModifier Modifier)
{
    Super::ApplyStatModifier(Modifier); // base class for damage, hp, etc.

    // player deals with economics
    const FGameplayTag Tag_CoinMult = FGameplayTag::RequestGameplayTag(FName("Stats.CoinMultiplier"));
    const FGameplayTag Tag_Gold = FGameplayTag::RequestGameplayTag(FName("Stats.Gold"));

    if (Modifier.StatTag.MatchesTagExact(Tag_CoinMult))
    {
        CoinMultiplier += Modifier.bIsMultiplier ? Modifier.Value * CoinMultiplier : Modifier.Value;
    }
    else if (Modifier.StatTag.MatchesTagExact(Tag_Gold))
    {
        int32 BonusGold = Modifier.bIsMultiplier ? FMath::FloorToInt(CoinCount * (Modifier.Value - 1.0f)) : FMath::RoundToInt(Modifier.Value);
        AddCoins(BonusGold);
    }
}

void ACPP_PlayerCharacter::MoveRight(float Value)
{
    if (bIsDead) return;
    FVector Direction = FVector(0.0f, 1.0f, 0.0f);
    AddMovementInput(Direction, Value);
}

void ACPP_PlayerCharacter::MoveForward(float Value)
{
    if (bIsDead) return;
    FVector Direction = FVector(1.0f, 0.0f, 0.0f);
    AddMovementInput(Direction, Value);
}

void ACPP_PlayerCharacter::StopDodge()
{
    bIsDodging = false;
    bIsInvulnerable = false;

    GetSprite()->SetSpriteColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}
void ACPP_PlayerCharacter::StopJump()
{
    bIsJumping = false;
}

void ACPP_PlayerCharacter::PrimaryAttack()
{
    if (IsDead()) return;

    if (GetIsAttacking())
    {
        if (ComboCounter < 2)
        {
            bInputBuffered = true;

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Input Buffered!"));
        }
    }
    else
    {
        if (GetWorldTimerManager().IsTimerActive(ComboResetTimer))
        {
            ComboCounter++;

            if (ComboCounter > 2)
            {
                ComboCounter = 0;
            }

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange,
                FString::Printf(TEXT("Combo Window Hit! Stage: %d"), ComboCounter));
        }
        else
        {
            ComboCounter = 0;

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("Fresh Combo Started"));
        }

        ExecuteMeleeAttack();
    }
}

void ACPP_PlayerCharacter::MagicAttack()
{
    if (!ActionComp) return;
    ActionComp->StartActionBySlot(this, FGameplayTag::RequestGameplayTag("Slot.Ability.R"));
}

void ACPP_PlayerCharacter::ExecuteMeleeAttack()
{
    if (!ActionComp || IsDead()) return;

    GetWorldTimerManager().ClearTimer(ComboResetTimer);

    FGameplayTag SelectedTag;

    switch (ComboCounter)
    {
    case 0:
        SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.LightAttack");
        break;
    case 1:
        SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.DashAttack");
        break;
    case 2:
        SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.HeavyAttack");

        if (CharacterStats && AttributeComp)
        {
            AttributeComp->SetBaseSpeed(CharacterStats->HeavyAttackWalkSpeed);
        }
        break;
    default:
        SelectedTag = FGameplayTag::RequestGameplayTag("Ability.Player.Melee.LightAttack");
        ComboCounter = 0;
        break;
    }

    bool bStarted = ActionComp->StartActionByName(this, SelectedTag);

    if (bStarted)
    {
        bInputBuffered = false;

        UCPP_Action* CurrentAction = ActionComp->GetAction(SelectedTag);
        if (CurrentAction)
        {
            CurrentAction->OnActionStopped.AddDynamic(this, &ACPP_PlayerCharacter::OnAttackActionStopped);
        }

        GetWorldTimerManager().ClearTimer(ComboResetTimer);
    }
    else
    {
        if (CharacterStats && GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = CharacterStats->MaxWalkSpeed;
        }
    }
}

void ACPP_PlayerCharacter::OnAttackActionStopped(UCPP_Action* Action)
{
    if (Action)
    {
        Action->OnActionStopped.RemoveDynamic(this, &ACPP_PlayerCharacter::OnAttackActionStopped);
    }

    if (CharacterStats && AttributeComp)
    {
        AttributeComp->SetBaseSpeed(CharacterStats->MaxWalkSpeed);
    }

    if (bInputBuffered)
    {
        ComboCounter++;

        if (ComboCounter > 2)
        {
            ComboCounter = 0;
        }

        ExecuteMeleeAttack();
    }
    else
    {
        float ResetTime = (CharacterStats) ? CharacterStats->ComboResetTime : 1.0f;
        GetWorldTimerManager().SetTimer(ComboResetTimer, this, &ACPP_PlayerCharacter::ResetCombo, ResetTime, false);
    }
}

void ACPP_PlayerCharacter::ResetCombo()
{
    ComboCounter = 0;
    bInputBuffered = false;

    if (CharacterStats && AttributeComp)
    {
        AttributeComp->SetBaseSpeed(CharacterStats->MaxWalkSpeed);
    }

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Combo Reset"));
}

void ACPP_PlayerCharacter::LevelUp()
{
    CharacterLevel++;

    XPToNextLevel = UCPP_ProgressionStatics::CalculateRequiredXPForNextLevel(XPToNextLevel, LevelUpMultiplier);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("LEVEL UP! Level: %d"), CharacterLevel));

    float HPBonus = CharacterStats->HealthGrowthPerLevel;
    float DmgBonus = CharacterStats->DamageGrowthPerLevel;

    if (CharacterStats)
    {
        HPBonus = CharacterStats->HealthGrowthPerLevel;
        DmgBonus = CharacterStats->DamageGrowthPerLevel;

        CurrentBaseDamage += DmgBonus;

        if (AttributeComp)
        {
            float NewMaxHP = AttributeComp->GetMaxHealth() + HPBonus;

            AttributeComp->InitializeStats(NewMaxHP);
        }

        FText LevelUpMsg = FText::Format(
            NSLOCTEXT("HUD", "LevelUpDetail", "Level Up! HP +{0}, Dmg +{1}"),
            FText::AsNumber((int32)HPBonus),
            FText::AsNumber((int32)DmgBonus)
        );
        ShowNotification(LevelUpMsg, FColor::Yellow);
    }

    // Broadcast called in InitializeStats -> OnHealthChangedCallback, 
}

void ACPP_PlayerCharacter::OnSaveGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject || !AttributeComp) return;

    SaveObject->Health = AttributeComp->GetHealth();
    SaveObject->MaxHealth = AttributeComp->GetMaxHealth();
    SaveObject->BaseDamage = CurrentBaseDamage;
    SaveObject->CurrentXP = CurrentXP;
    SaveObject->XPToNextLevel = XPToNextLevel;
    SaveObject->Level = CharacterLevel;
    SaveObject->Coins = CoinCount;
    SaveObject->PlayerLocation = GetActorLocation();
    SaveObject->LevelName = FName(*GetWorld()->GetName());
    if (ActionComp)
    SaveObject->AbilityLevels = ActionComp->GetAllActionLevels();

    if (UCPP_InventoryComponent* InvComp = FindComponentByClass<UCPP_InventoryComponent>())
    {
        SaveObject->InventoryData = InvComp->GetInventory();
    }
}

void ACPP_PlayerCharacter::OnLoadGame_Implementation(UCPP_SaveGame* SaveObject)
{
    if (!SaveObject || !AttributeComp) return;

    AttributeComp->InitializeStats(SaveObject->MaxHealth);
    float Diff = SaveObject->Health - AttributeComp->GetHealth();
    AttributeComp->ApplyHealthChange(nullptr, Diff);

    CurrentBaseDamage = SaveObject->BaseDamage;
    CharacterLevel = SaveObject->Level;
    CurrentXP = SaveObject->CurrentXP;
    XPToNextLevel = (SaveObject->XPToNextLevel > 0) ? SaveObject->XPToNextLevel : 100.0f;
    CoinCount = SaveObject->Coins;
    if (ActionComp)
    ActionComp->RestoreActionLevels(SaveObject->AbilityLevels);

    //if (ActionComp && ActionComp->GetActionSet())
    //{
    //    UCPP_ActionSet* CurrentSet = ActionComp->GetActionSet();

    //    for (auto& It : AbilityLevels)
    //    {
    //        FGameplayTag SavedTag = It.Key;
    //        int32 SavedLevel = It.Value;

    //        if (SavedLevel > 0)
    //        {
    //            if (ActionComp->GetAction(SavedTag) != nullptr) continue;

    //            for (TSubclassOf<UCPP_Action> ActionClass : CurrentSet->Actions)
    //            {
    //                if (ActionClass)
    //                {
    //                    UCPP_Action* DefaultObj = ActionClass->GetDefaultObject<UCPP_Action>();
    //                    if (DefaultObj && DefaultObj->ActionTag == SavedTag)
    //                    {
    //                        ActionComp->AddAction(ActionClass);

    //                        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
    //                            FString::Printf(TEXT("Restored Ability: %s (Level %d)"), *SavedTag.ToString(), SavedLevel));

    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}

    SetActorLocation(SaveObject->PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);

    if (UCPP_InventoryComponent* InvComp = FindComponentByClass<UCPP_InventoryComponent>())
    {
        InvComp->SetInventory(SaveObject->InventoryData);
    }

    OnXPUpdated.Broadcast(CurrentXP, XPToNextLevel, CharacterLevel);
    OnCoinsUpdated.Broadcast(CoinCount);
}

void ACPP_PlayerCharacter::InputAbilityQ() {
    ActionComp->StartActionBySlot(this, FGameplayTag::RequestGameplayTag("Slot.Ability.Q"));
}

void ACPP_PlayerCharacter::InputAbilityE() {
    ActionComp->StartActionBySlot(this, FGameplayTag::RequestGameplayTag("Slot.Ability.E"));
}

void ACPP_PlayerCharacter::InputAbilityR() {
    ActionComp->StartActionBySlot(this, FGameplayTag::RequestGameplayTag("Slot.Ability.R"));
}

void ACPP_PlayerCharacter::ModifyStat_Implementation(FStatModifier Modifier)
{
    ApplyStatModifier(Modifier);
}

void ACPP_PlayerCharacter::AddResource_Implementation(FGameplayTag ResourceTag, float Amount)
{
    if (ResourceTag.MatchesTag(FGameplayTag::RequestGameplayTag("Resource.Gold")))
    {
        AddCoins(FMath::RoundToInt(Amount));
    }
    else if (ResourceTag.MatchesTag(FGameplayTag::RequestGameplayTag("Resource.XP")))
    {
        AddExperience(Amount);
    }
}
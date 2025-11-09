// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseCharacter.h"
#include "Components/InputComponent.h"
#include "PaperFlipbookComponent.h" //sprite component
#include "Engine/Engine.h"


ACPP_BaseCharacter::ACPP_BaseCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

    // Получаем доступ к компоненту движения
    UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

    // Устанавливаем скорость бега
    MoveComponent->MaxWalkSpeed = 450.0f;

    // Устанавливаем начальную скорость прыжка
    MoveComponent->JumpZVelocity = 450.0f;

    // Устанавливаем гравитацию (если нужно)
    MoveComponent->GravityScale = 1.f;

    // Также можно настроить возможность управления в воздухе
    MoveComponent->AirControl = 0.5f; // 0 = нет контроля, 1 = полный контроль
}

// Эта функция соединяет названия осей ("MoveRight") с нашими C++ функциями
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

    //attack
    PlayerInputComponent->BindAction("AttackAction", 
        IE_Pressed, this, &ACPP_BaseCharacter::Attack);
}

void ACPP_BaseCharacter::MoveRight(float Value)
{
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
    AddMovementInput(GetActorForwardVector(), Value);
}

void ACPP_BaseCharacter::Dodge()
{

    if (bIsDodging) return;
    bIsDodging = true;
    bIsInAir = true;
    float DodgeDuration = 0.5f;

    if (DodgeAnimationFlipbook)
    {
        GetSprite()->SetFlipbook(DodgeAnimationFlipbook);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("DODGE anim CALLED!"));
    }
    
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("DODGE FUNCTION CALLED!"));
    FVector RightDirection = GetActorRightVector();

    // 2. Проверяем, отражен ли спрайт.
    // Если Scale.X отрицательный, значит, персонаж смотрит "назад" относительно актера.
    if (GetSprite()->GetRelativeScale3D().X < 0.0f)
    {
        // 3. Инвертируем направление, чтобы оно соответствовало спрайту.
        RightDirection *= -1.0f;
    }
    const FVector DodgeHorizontalDirection = RightDirection * -1.0f;
    const FVector UpDirection = FVector::UpVector;
    FVector CombinedDirection = DodgeHorizontalDirection + UpDirection*0.5;
    CombinedDirection.Normalize();

    const FVector DodgeDirection = RightDirection * -1.0f;

    const float DodgeStrength = 500.0f;
    LaunchCharacter(CombinedDirection * DodgeStrength, true, false);

    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this, 
        &ACPP_BaseCharacter::StopDodge, DodgeDuration, false);
}

void ACPP_BaseCharacter::StopDodge()
{
    bIsDodging = false;
}
void ACPP_BaseCharacter::StopJump()
{
    bIsJumping = false;
}

void ACPP_BaseCharacter::Attack()
{
    // Если мы уже атакуем, ничего не делаем
    if (bIsAttacking) return;

    // Увеличиваем счетчик комбо
    ComboCounter++;
    bIsAttacking = true;

    // Сбрасываем предыдущий таймер сброса комбо, если он был
    GetWorldTimerManager().ClearTimer(ComboResetTimer);

    float AttackDuration = 0.5f; // Длительность по умолчанию

    FVector RightDirection = GetActorRightVector();
    if (GetSprite()->GetRelativeScale3D().X < 0.0f)
    {
        RightDirection *= -1.0f;
    }

    switch (ComboCounter)
    {
    case 1:
    {
        // --- ПЕРВАЯ АТАКА (БАЗОВАЯ) ---
        if (Attack1Flipbook)
        {
            GetSprite()->SetFlipbook(Attack1Flipbook);
        }
        AttackDuration = 0.2f;

        // Определяем параметры трассировки
        const FVector Start = GetActorLocation(); // Начинаем от себя
        const FVector End = Start + (RightDirection * 150.0f); // "Выпускаем" на 150 юнитов вперед
        const FVector HalfSize = FVector(50.0f, 50.0f, 50.0f); // Размер "коробки" урона
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.Add(this); // Игнорируем самих себя
        FHitResult HitResult;

        // Выполняем трассировку
        bool bHit = UKismetSystemLibrary::BoxTraceSingle(
            GetWorld(),
            Start,
            End,
            HalfSize,
            GetActorRotation(),
            UEngineTypes::ConvertToTraceType(ECC_WorldDynamic),
            false,
            ActorsToIgnore,
            EDrawDebugTrace::ForDuration, // Рисуем коробку для отладки
            HitResult,
            true
        );

        // Если мы в кого-то попали
        if (bHit)
        {
            AActor* HitActor = HitResult.GetActor();
            if (HitActor)
            {
                // Наносим урон
                float DamageToApply = 25.0f; // Базовый урон
                if (ComboCounter == 3) DamageToApply = 50.0f; // Усиленный урон для 3-го удара

                // Вызываем стандартную функцию нанесения урона
                UGameplayStatics::ApplyDamage(
                    HitActor, // Кому наносим урон
                    DamageToApply, // Сколько урона
                    GetController(), // Кто нанес урон (контроллер)
                    this, // Сам "виновник" урона (наш персонаж)
                    UDamageType::StaticClass() // Тип урона (пока стандартный)
                );
            }
        }

        break;
    }
    case 2:
        // --- ВТОРАЯ АТАКА (С РЫВКОМ) ---
        if (Attack2Flipbook)
        {
            GetSprite()->SetFlipbook(Attack2Flipbook);
        }
        // Добавляем рывок вперед
        LaunchCharacter(RightDirection * 200.0f, true, true);
        AttackDuration = 0.5f;
        break;

    case 3:
        // --- ТРЕТЬЯ АТАКА (ТЯЖЕЛАЯ) ---
        if (Attack3Flipbook)
        {
            GetSprite()->SetFlipbook(Attack3Flipbook);
        }
        // Ограничиваем движение
        GetCharacterMovement()->MaxWalkSpeed = 50.0f; // Сильно замедляем
        AttackDuration = 0.6f;
        break;

    default:
        // Если что-то пошло не так, сбрасываем
        ResetCombo();
        return;
    }

    // Ставим таймер на завершение текущей атаки
    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this, 
        &ACPP_BaseCharacter::AttackEnd, AttackDuration, false);
}

void ACPP_BaseCharacter::AttackEnd()
{
    // Атака завершена, мы снова можем атаковать
    bIsAttacking = false;

    // Восстанавливаем скорость после тяжелой атаки, если это был третий удар
    if (ComboCounter == 3)
    {
        // Возвращаем скорость к значению по умолчанию, которое мы задали в конструкторе
        GetCharacterMovement()->MaxWalkSpeed = 450.0f;
    }

    // Ставим таймер, который сбросит комбо, если не будет следующего удара
    // 0.3f - это "окно" для следующего удара в комбо
    GetWorldTimerManager().SetTimer(ComboResetTimer, this,
        &ACPP_BaseCharacter::ResetCombo, 0.3f, false);
}

void ACPP_BaseCharacter::ResetCombo()
{
    // Сбрасываем все в исходное состояние
    ComboCounter = 0;
    bIsAttacking = false;
    // На всякий случай восстанавливаем скорость, если комбо прервалось
    GetCharacterMovement()->MaxWalkSpeed = 450.0f;
}

void ACPP_BaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
}

void ACPP_BaseCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    if (bIsJumping) return;
    bIsJumping = true;
    bIsInAir = true;
    float JumpDuration=0.5f;

    if (JumpAnimationFlipbook)
    {
        GetSprite()->SetFlipbook(JumpAnimationFlipbook);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("jump anim called"));
    }

    FTimerHandle UnusedHandle;
    GetWorldTimerManager().SetTimer(UnusedHandle, this,
        &ACPP_BaseCharacter::StopJump, JumpDuration, false);
}

float ACPP_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Сначала вызываем родительскую функцию, чтобы получить базовый обработанный урон
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Уменьшаем здоровье
    CurrentHealth -= ActualDamage;

    // Выводим в лог для отладки
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("%s took %f damage. Current Health: %f"), *GetName(), ActualDamage, CurrentHealth));
    }

    // Проверяем, не умер ли персонаж
    if (CurrentHealth <= 0.0f)
    {
        // Здесь будет логика смерти
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s has died!"), *GetName()));
        }
        // Пока просто уничтожим актера
        Destroy();
    }

    return ActualDamage;
}

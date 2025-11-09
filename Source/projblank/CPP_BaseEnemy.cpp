// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseEnemy.h"
#include "Perception/PawnSensingComponent.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "PaperFlipbookComponent.h"

ACPP_BaseEnemy::ACPP_BaseEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Создаем компонент "зрения"
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

    // Настраиваем его. Эти параметры можно будет менять в Блюпринтах.
    PawnSensingComp->SetPeripheralVisionAngle(60.0f); // Угол обзора
    PawnSensingComp->SightRadius = 800.0f;           // Дальность обзора
    PawnSensingComp->HearingThreshold = 600.0f;      // Дальность "слуха"
    PawnSensingComp->LOSHearingThreshold = 1200.0f;  // Дальность "слуха" в прямой видимости

    // Настраиваем скорость врага
    GetCharacterMovement()->MaxWalkSpeed = 250.0f;

    // AIControllerClass = AIC_BaseEnemy::StaticClass();

    PrimaryActorTick.bCanEverTick = true;
}

// Эта функция будет вызываться, когда враг "увидит" игрока
void ACPP_BaseEnemy::OnPawnSeen(APawn* SeenPawn)
{
    if (SeenPawn == nullptr)
    {
        return;
    }

    // Выводим отладочное сообщение
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("I see you, %s!"), *SeenPawn->GetName()));
    }

    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        // Записываем увиденную пешку в Blackboard
        AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", SeenPawn);
    }


}

void ACPP_BaseEnemy::BeginPlay()
{
    Super::BeginPlay();

    // Привязываем нашу функцию к событию OnSeePawn
    if (PawnSensingComp)
    {
        PawnSensingComp->OnSeePawn.AddDynamic(this, &ACPP_BaseEnemy::OnPawnSeen);
    }
}

void ACPP_BaseEnemy::Tick(float DeltaTime)
{
    // ВАЖНО: Вызываем Tick родительского класса!
    // Это выполнит всю логику Tick из ACPP_BaseCharacter (если она там есть).
    Super::Tick(DeltaTime);

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, TEXT("Enemy Tick is Running!"));

    // Получаем текущую скорость врага
    const FVector Velocity = GetVelocity();
    // Получаем вектор "вправо" от врага
    
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("Velocity Length: %f"), Velocity.Size()));
    
    const FVector RightVector = GetActorRightVector();

    // Проверяем, что враг вообще движется. Если он стоит, не меняем направление.
    if (!Velocity.IsNearlyZero())
    {
        // Скалярное произведение векторов.
        // Если векторы сонаправлены (враг движется вправо), результат > 0.
        // Если противонаправлены (враг движется влево), результат < 0.
        const float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), RightVector);
        
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Dot Product: %f"), DotProduct));
        // Получаем текущий масштаб, чтобы изменить только ось X
        FVector CurrentScale = GetSprite()->GetRelativeScale3D();

        if (Velocity.Y > 0.1f) // Движемся вправо по карте
        {
            CurrentScale.X = FMath::Abs(CurrentScale.X);
        }
        else if (Velocity.Y < -0.1f) // Движемся влево по карте
        {
            CurrentScale.X = -FMath::Abs(CurrentScale.X);
        }

        //if (DotProduct > 0.1f) // Движемся вправо
        //{
        //    CurrentScale.Y = FMath::Abs(CurrentScale.Y); // Делаем масштаб положительным
        //}
        //else if (DotProduct < -0.1f) // Движемся влево
        //{
        //    CurrentScale.Y = -FMath::Abs(CurrentScale.Y); // Делаем масштаб отрицательным
        //}
        // Если DotProduct близок к нулю (движемся строго вперед/назад), ничего не делаем

        GetSprite()->SetRelativeScale3D(CurrentScale);
    }
}

void ACPP_BaseEnemy::AttackPlayer()
{
    if (bIsAttacking) return;

    bIsAttacking = true;

    if (EnemyAttackFlipbook)
    {
        GetSprite()->SetFlipbook(EnemyAttackFlipbook);
    }

    // --- ЛОГИКА НАНЕСЕНИЯ УРОНА ИГРОКУ ---

    // Запускаем трассировку через небольшую задержку, чтобы она совпала с анимацией удара
    FTimerHandle HitTraceTimer;
    GetWorldTimerManager().SetTimer(HitTraceTimer, this, &ACPP_BaseEnemy::PerformHitTrace, 0.2f, false);
}

void ACPP_BaseEnemy::PerformHitTrace()
{
    const FVector Start = GetActorLocation();
    const FVector End = Start + (GetActorForwardVector() * 150.0f); // Направление зависит от поворота AI
    const FVector HalfSize = FVector(60.0f, 60.0f, 60.0f);
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    FHitResult HitResult;

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
    ); // Скопируйте параметры из атаки игрока

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        // Проверяем, что попали именно в игрока (по тегу или классу)
        ACPP_BaseCharacter* PlayerCharacter = Cast<ACPP_BaseCharacter>(HitActor);
        if (PlayerCharacter && !Cast<ACPP_BaseEnemy>(PlayerCharacter)) // Убеждаемся, что это игрок, а не другой враг
        {
            float DamageToApply = 15.0f;
            UGameplayStatics::ApplyDamage(HitActor, DamageToApply, GetController(), this, UDamageType::StaticClass());
        }
    }

    // После выполнения трассировки завершаем атаку
    FinishAttack();
}

void ACPP_BaseEnemy::FinishAttack()
{
    bIsAttacking = false;
    // Здесь можно будет вернуть Idle анимацию, но пока наша логика в Tick с этим справится
}
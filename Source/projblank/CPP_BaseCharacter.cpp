// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseCharacter.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    // 4. Для уклонения берем "истинное" направление взгляда и инвертируем его.
    // Это и будет направление "назад" от персонажа.
    const FVector DodgeDirection = RightDirection * -1.0f;

    const float DodgeStrength = 500.0f;
    LaunchCharacter(CombinedDirection * DodgeStrength, true, false);
}
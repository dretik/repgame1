// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BaseCharacter.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h" //sprite component

// Эта функция соединяет названия осей ("MoveRight") с нашими C++ функциями
void ACPP_BaseCharacter::SetupPlayerInputComponent(UInputComponent* 
    PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Привязываем ось "MoveRight" к нашей функции MoveRight
    PlayerInputComponent->BindAxis("MoveRight", this, 
        &ACPP_BaseCharacter::MoveRight);
    // Привязываем ось "MoveForward" к нашей функции MoveForward
    PlayerInputComponent->BindAxis("MoveForward", this, 
        &ACPP_BaseCharacter::MoveForward);
}

// Эта функция вызывается, когда мы нажимаем A или D
void ACPP_BaseCharacter::MoveRight(float Value)
{
    // Добавляем движение. Это полный аналог узла "Add Movement Input"
    AddMovementInput(GetActorRightVector(), Value);

    // --- Наша логика поворота спрайта ---
    // Value здесь - это зеленый пин Axis Value из Блюпринта
    if (Value > 0.0f)
    {
        // Смотрим вправо
        GetSprite()->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
    }
    else if (Value < 0.0f)
    {
        // Смотрим влево
        GetSprite()->SetRelativeScale3D(FVector(-1.0f, 1.0f, 1.0f));
    }
    // Если Value == 0.0f, мы ничего не делаем, и масштаб остается прежним
}

// Эта функция вызывается, когда мы нажимаем W или S
void ACPP_BaseCharacter::MoveForward(float Value)
{
    AddMovementInput(GetActorForwardVector(), Value);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_DamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

// Sets default values
ACPP_DamageTextActor::ACPP_DamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;
	DamageWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidgetComp"));
	RootComponent = DamageWidgetComp;

	DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComp->SetDrawAtDesiredSize(true);
}

// Called when the game starts or when spawned
void ACPP_DamageTextActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeTime);
	
}

void ACPP_DamageTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewLoc = GetActorLocation();
	NewLoc.Z += 50.0f * DeltaTime;
	SetActorLocation(NewLoc);
}

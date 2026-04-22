#include "CPP_Action.h"
#include "CPP_ActionComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

bool UCPP_Action::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	UCPP_ActionComponent* Comp = GetOwningComponent();

	static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag("Status.Stunned");

	if (Comp)
	{
		if (Comp->ActiveGameplayTags.HasTag(StunTag))
		{
			return false;
		}
		//stun tag realisation
		//UCPP_ActionComponent* ActionComp = Target->FindComponentByClass<UCPP_ActionComponent>();
		//if (ActionComp)
		//{
		//	FGameplayTag StunTag = FGameplayTag::RequestGameplayTag("Status.Stunned");
		//	ActionComp->ActiveGameplayTags.AddTag(StunTag);

		//
		//}
		FGameplayTag BlockingTag = FGameplayTag::RequestGameplayTag("Ability");

		if (Comp->ActiveGameplayTags.HasTag(BlockingTag))
		{
			// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Action Blocked: Actor is Busy!"));
			return false;
		}
	}

	float TimeSinceStart = GetWorld()->GetTimeSeconds() - TimeStarted;
	if (TimeSinceStart < CooldownTime)
	{
		return false;
	}

	return true;
}

void UCPP_Action::StartAction_Implementation(AActor* Instigator)
{
	// Log: Action Started
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Action Started: %s"), *GetName()));

	float FinalDuration = Duration;
	
	UCPP_ActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->ActiveGameplayTags.AddTag(ActionTag);
	}

	if (FinalDuration <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("Action '%s' started with 0 Duration! Forcing safety fallback."), *GetName());

		ensureMsgf(false, TEXT("Action %s has 0 duration! Check Blueprint settings."), *GetName());

		FinalDuration = MinActionDuration;
	}

	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;
	TimeStarted = GetWorld()->GetTimeSeconds();

	if (Duration > 0.0f)
	{
		FTimerDelegate StopDel;
		StopDel.BindUFunction(this, "StopAction", Instigator);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AutoStop, StopDel, Duration, false);
	}
}

void UCPP_Action::StopAction_Implementation(AActor* Instigator)
{
	// Log: Action Stopped
	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, FString::Printf(TEXT("Action Stopped: %s"), *GetName()));
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AutoStop);
	}

	UCPP_ActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->ActiveGameplayTags.RemoveTag(ActionTag);
	}

	RepData.bIsRunning = false;
	RepData.Instigator = nullptr;

	if (OnActionStopped.IsBound())
	{
		OnActionStopped.Broadcast(this);
	}
}

void UCPP_Action::RefreshAction_Implementation(AActor* Instigator)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Action Refreshed: %s"), *GetName()));

	TimeStarted = GetWorld()->GetTimeSeconds();

	if (Duration > 0.0f)
	{
		FTimerDelegate StopDel;
		StopDel.BindUFunction(this, "StopAction", Instigator);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AutoStop, StopDel, Duration, false);
	}
}

UWorld* UCPP_Action::GetWorld() const
{
	//taking from outer (component)
	if (UCPP_ActionComponent* Comp = Cast<UCPP_ActionComponent>(GetOuter()))
	{
		return Comp->GetWorld();
	}
	return nullptr;
}

UCPP_ActionComponent* UCPP_Action::GetOwningComponent() const
{
	return Cast<UCPP_ActionComponent>(GetOuter());
}

bool UCPP_Action::IsRunning() const
{
	return RepData.bIsRunning;
}

float UCPP_Action::GetCooldownRemaining() const {
	float TimeSinceStart = GetWorld()->GetTimeSeconds() - TimeStarted;
	return FMath::Max(0.0f, CooldownTime - TimeSinceStart);
}

float UCPP_Action::GetCooldownNormalized() const {
	if (CooldownTime <= 0.0f) return 0.0f;
	return GetCooldownRemaining() / CooldownTime;
}
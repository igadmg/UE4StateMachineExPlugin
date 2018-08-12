#include "StateMachineExPrivatePCH.h"

#include "StateMachine/State.h"
#include "StateMachine/StateMachine.h"

#include "Kismet/GameplayStatics.h"



UState::UState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UState::GetWorld() const
{
	return (!HasAnyFlags(RF_ClassDefaultObject) && GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

APlayerController* UState::GetOwningPlayer() const
{
	return UGameplayStatics::GetPlayerController(this, 0);
}

APawn* UState::GetOwningPlayerPawn() const
{
	const APlayerController* PlayerController = GetOwningPlayer();

	return IsValid(PlayerController)
		? PlayerController->GetPawnOrSpectator()
		: nullptr;
}

void UState::Enter_Implementation()
{
}

void UState::Tick_Implementation(float DeltaTime)
{
}

void UState::Exit_Implementation()
{
}

void UState::Restart_Implementation()
{
	ParentStateMachine->SwitchState(GetClass());
}

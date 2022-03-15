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

UObject* UState::ConstructState_Implementation(UStateMachine* StateMachine)
{
	UState* NewState = NewObject<UState>(StateMachine, GetClass());
	NewState->ParentStateMachine = StateMachine;

	return NewState;
}

void UState::EnterState_Implementation()
{
	Status = EStateStatus::Entered;

	UE_LOG(LogStateMachineEx, Verbose, TEXT("Entering state %s State Machine %s"), *GetClass()->GetName(), *ParentStateMachine->GetClass()->GetName());
}

void UState::TickState_Implementation(float DeltaTime)
{
	Status = EStateStatus::Updated;
}

void UState::ExitState_Implementation()
{
	Status = EStateStatus::Exited;

	UE_LOG(LogStateMachineEx, Verbose, TEXT("Exiting state %s State Machine %s"), *GetClass()->GetName(), *ParentStateMachine->GetClass()->GetName());
}

void UState::Restart_Implementation()
{
	Status = EStateStatus::Restarted;
	ParentStateMachine->SwitchStateByClass(GetClass());
}

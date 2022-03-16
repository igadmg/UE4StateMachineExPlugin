#include "StateMachine/StateMachineState.h"

#include "StateMachine/StateMachine.h"



UStateMachineState::UStateMachineState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UStateMachineState::EnterState_Implementation()
{
	Super::EnterState_Implementation();

	InternalStateMachine = NewObject<UStateMachine>(this);
}

void UStateMachineState::TickState_Implementation(float DeltaSeconds)
{
	Super::TickState_Implementation(DeltaSeconds);

	if (InternalStateMachine)
		InternalStateMachine->Tick(DeltaSeconds);
}

void UStateMachineState::ExitState_Implementation()
{
	if (InternalStateMachine)
		InternalStateMachine->Shutdown();

	Super::ExitState_Implementation();
}



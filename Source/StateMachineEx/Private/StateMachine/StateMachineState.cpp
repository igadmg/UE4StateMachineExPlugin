#include "StateMachine/StateMachineState.h"

#include "StateMachine/StateMachine.h"



UStateMachineState::UStateMachineState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UStateMachineState::EnterState_Implementation()
{
	Super::EnterState_Implementation();

	StateMachine = NewObject<UStateMachine>(this);
}

void UStateMachineState::TickState_Implementation(float DeltaSeconds)
{
	Super::TickState_Implementation(DeltaSeconds);

	if (StateMachine)
		StateMachine->Tick(DeltaSeconds);
}

void UStateMachineState::ExitState_Implementation()
{
	if (StateMachine)
		StateMachine->Shutdown();

	Super::ExitState_Implementation();
}



#include "Blueprint/CreateStateAsyncTask.h"

#include "StateMachine/StateMachine.h"
#include "StateMachine/State.h"
#include "StateMachineExStatics.h"



UCreateStateAsyncTask::UCreateStateAsyncTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UState* UCreateStateAsyncTask::CreateStateObject(UObject* WorldContextObject, UClass* StateClass)
{
	if (!IsValid(WorldContextObject) || !IsValid(StateClass))
		return nullptr;

	UStateMachine* StateMachine = UStateMachineExStatics::GuessStateMachine(WorldContextObject);
	if (!IsValid(StateMachine))
		return nullptr;

	return StateMachine->SwitchState(StateClass);
}


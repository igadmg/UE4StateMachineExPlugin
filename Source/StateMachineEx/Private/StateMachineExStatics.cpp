#include "StateMachineExStatics.h"

#include "StateMachine/StateMachine.h"
#include "StateMachine/StateMachineState.h"
#include "StateMachine/State.h"

#include "CoreEx.h"



UStateMachine* UStateMachineExStatics::SpawnStateMachine(UObject* Owner, TSubclassOf<UStateMachine> StateMachineClass)
{
	return NewObject<UStateMachine>(Owner, StateMachineClass);
}

UStateMachine* UStateMachineExStatics::GuessStateMachineInternal(UObject* WorldContextObject)
{
	if (auto StateMachine = Valid<UStateMachine>(WorldContextObject))
		return StateMachine;

	if (auto StateMachineState = Valid<UStateMachineState>(WorldContextObject))
		return StateMachineState->InternalStateMachine;

	for (auto* ObjecProperty : TFieldRange<FObjectProperty>(WorldContextObject->GetClass()))
	{
		if (ObjecProperty->PropertyClass == UStateMachine::StaticClass()
			|| ObjecProperty->PropertyClass->IsChildOf(UStateMachine::StaticClass()))
		{
			auto StateMachine = Cast<UStateMachine>(ObjecProperty->GetPropertyValue_InContainer(WorldContextObject));

			if (!IsValid(StateMachine))
			{
				StateMachine = NewObject<UStateMachine>(WorldContextObject, ObjecProperty->PropertyClass);
				if (auto AsActor = Cast<AActor>(WorldContextObject))
				{
					UE_LOG_EX_WCO(LogStateMachineExCriticalErrors, Error, WorldContextObject, TEXT("Automatic StateMachine ticking is not yet supported."))
					//AsActor->Tick
				}
			}

			return StateMachine;
		}
	}

	return nullptr;
}

UObject* UStateMachineExStatics::GuessCurrentStateInternal(UObject* WorldContextObject)
{
	if (auto StateMachine = Valid(GuessStateMachineInternal(WorldContextObject)))
	{
		return StateMachine->CurrentState;
	}

	return nullptr;
}

void UStateMachineExStatics::PushState(UObject* WorldContextObject)
{
	UStateMachine* StateMachine = GuessStateMachineInternal(WorldContextObject);
	if (!IsValid(StateMachine))
		return;

	if (!IsValid(StateMachine->CurrentState))
		return;

	StateMachine->StateStack.Push(StateMachine->CurrentState);
}

void UStateMachineExStatics::PopState(UObject* WorldContextObject)
{
	UStateMachine* StateMachine = GuessStateMachineInternal(WorldContextObject);
	if (!IsValid(StateMachine))
		return;

	if (StateMachine->StateStack.Num() > 0)
	{
		StateMachine->SwitchState(StateMachine->StateStack.Pop());
	}
	else
	{
		StateMachine->SwitchState(nullptr);
	}
}

UObject* UStateMachineExStatics::CreateStateObject(UObject* WorldContextObject, UClass* StateClass)
{
	if (!IsValid(WorldContextObject) || !IsValid(StateClass))
		return nullptr;

	UStateMachine* StateMachine = UStateMachineExStatics::GuessStateMachineInternal(WorldContextObject);
	if (!IsValid(StateMachine))
	{
		UE_LOG(LogStateMachineExCriticalErrors, Error, TEXT("State %s is spawn without State Machine context in object %s"), *StateClass->GetClass()->GetName(), *WorldContextObject->GetClass()->GetName());

		return nullptr;
	}

	return StateMachine->PrepareState(StateClass);
}

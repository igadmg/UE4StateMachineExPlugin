#include "StateMachineExStatics.h"

#include "StateMachine/StateMachine.h"
#include "StateMachine/State.h"

#include "StateMachineEx.final.h"


UStateMachine* UStateMachineExStatics::SpawnStateMachine(UObject* Owner, TSubclassOf<UStateMachine> StateMachineClass, bool bAutoTick)
{
	if (auto StateMachine = NewObject<UStateMachine>(Owner, StateMachineClass))
	{
		StateMachine->AutoTickFunction.bCanEverTick = bAutoTick;

		return StateMachine;
	}

	return nullptr;
}

UStateMachine* UStateMachineExStatics::GuessStateMachineInternal(UObject* WorldContextObject)
{
	auto StateMachine = Valid<UStateMachine>(WorldContextObject);

	if (!IsValid(StateMachine))
	{
		for (auto* ObjecProperty : TFieldRange<FObjectProperty>(WorldContextObject->GetClass()))
		{
			if (ObjecProperty->PropertyClass->IsChildOf(UStateMachine::StaticClass()) && !ObjecProperty->HasAnyPropertyFlags(CPF_Transient))
			{
				StateMachine = Cast<UStateMachine>(ObjecProperty->GetPropertyValue_InContainer(WorldContextObject));

				if (!IsValid(StateMachine))
				{
					StateMachine = SpawnStateMachine(WorldContextObject, ObjecProperty->PropertyClass, true);
				}
	
				ObjecProperty->SetPropertyValue_InContainer(WorldContextObject, StateMachine);
				break;
			}
		}
	}

	if (!IsValid(StateMachine))
		return nullptr;

	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		if (StateMachine->AutoTickFunction.bCanEverTick)
		{
			if (auto AsActor = Cast<AActor>(WorldContextObject))
			{
				StateMachine->AutoTickFunction.TickGroup = AsActor->PrimaryActorTick.TickGroup;
				StateMachine->AutoTickFunction.AddPrerequisite(AsActor, AsActor->PrimaryActorTick);
			}
			else if (auto AsActorComponent = Cast<UActorComponent>(WorldContextObject))
			{
				StateMachine->AutoTickFunction.TickGroup = AsActorComponent->PrimaryComponentTick.TickGroup;
				StateMachine->AutoTickFunction.AddPrerequisite(AsActorComponent, AsActorComponent->PrimaryComponentTick);
			}
			//else if (auto AsWidget = Cast<UUserWidget>(WorldContextObject))
			//{
			//	StateMachine->AutoTickFunction.AddPrerequisite(AsActorComponent, AsActorComponent->PrimaryComponentTick);
			//}
			StateMachine->AutoTickFunction.RegisterTickFunction(World->PersistentLevel);
		}
	}

	return StateMachine;
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

UObject* UStateMachineExStatics::EmbedStateObject(UObject* WorldContextObject, UClass* StateClass)
{
	if (auto OwnerState = Valid<UState>(WorldContextObject))
	{
		// Other option is to find outer state machine, but it seems not right to use it, becuase embedded state is not executed by state machine.
		// UStateMachine* StateMachine = WorldContextObject->GetTypedOuter<UStateMachine>();
		// if (auto State = IStateInterface::Execute_ConstructState(GetMutableDefault<UObject>(StateClass), StateMachine))
		if (auto State = NewObject<UObject>(WorldContextObject, StateClass))
		{
			IStateInterface::Execute_EnterState(State);

			OwnerState->OnStateTick.AddWeakLambda(State, [State](UState* OwnerState, float DeltaSeconds) { IStateInterface::Execute_TickState(State, DeltaSeconds); });
			OwnerState->OnStateExit.AddWeakLambda(State, [State](UState* OwnerState) {
				IStateInterface::Execute_ExitState(State);
				State->ConditionalBeginDestroy();
			});

			return State;
		}
	}

	return nullptr;
}

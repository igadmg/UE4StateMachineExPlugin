#include "StateMachineExPrivatePCH.h"
#include "StateMachine/StateMachine.h"

#include "StateMachine/State.h"



UStateMachine::UStateMachine(const FObjectInitializer &Initializer)
	: Super(Initializer)
	, ShutdownState(nullptr)
	, CurrentState(nullptr)
	, NextState(nullptr)
{
}

UWorld* UStateMachine::GetWorld() const
{
	return (!HasAnyFlags(RF_ClassDefaultObject) && GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

bool UStateMachine::IsActive() const
{
	return IsValid(CurrentState) || IsValid(NextState);
}

UState* UStateMachine::SwitchState(TSubclassOf<UState> NewStateClass)
{
	UState* NewState = NewObject<UState>(this, NewStateClass);
	NewState->ConstructState(this);

	return SwitchState(NewState);
}

UState* UStateMachine::SwitchState(UState* NewState)
{
	if (IsValid(CurrentState))
	{
		CurrentState->Exit();
	}

	CurrentState = nullptr;
	NextState = NewState;

	return NextState;
}

void UStateMachine::Restart()
{
	Shutdown();
	Reset();
}

void UStateMachine::Reset_Implementation()
{
}

void UStateMachine::Tick_Implementation(float DeltaSeconds)
{
	while (!IsValid(CurrentState))
	{
		if (!IsValid(NextState))
		{
			// The state machine is shutdown and should be Reset.
			return;
		}

		UE_LOG(LogStateMachineEx, Log, TEXT("State Machine %s switched to state %s."), *GetClass()->GetName(), *NextState->GetClass()->GetName());

		CurrentState = NextState;
		NextState = nullptr;
		CurrentState->Enter();
	}

	if (!CurrentState->bPaused)
	{
		CurrentState->Tick(DeltaSeconds);
	}
}

void UStateMachine::Shutdown_Implementation()
{
	if (IsValid(CurrentState))
	{
		if (IsValid(ShutdownState))
		{
			SwitchState(ShutdownState);
			Tick(GetWorld() ? GetWorld()->GetDeltaSeconds() : 0);
		}
		if (IsValid(CurrentState))
		{
			CurrentState->Exit();
		}
	}

	CurrentState = nullptr;
	NextState = nullptr;
}

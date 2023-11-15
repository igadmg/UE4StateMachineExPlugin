#include "StateMachine/StateMachine.h"

#include "StateMachine/StateInterface.h"
#include "StateMachine/State.h"

#include "StateMachineEx.final.h"



void FStateMachineTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	//QUICK_SCOPE_CYCLE_COUNTER(FSkeletalMeshComponentEndPhysicsTickFunction_ExecuteTick);
	//CSV_SCOPED_TIMING_STAT_EXCLUSIVE(Physics);

	//TODO: add TimeDilation and other stuff here.
	//FActorComponentTickFunction::ExecuteTickHelper(Target, /*bTickInEditor=*/ false, DeltaTime, TickType, [this](float DilatedTime)
	if (IsValid(Target) && !Target->IsUnreachable())
	{
#if WITH_EDITOR
		auto IsAutoTickFunctionGuard = MakeGuardValue(Target->bIsAutoTickFunction, true);
#endif
		Target->Tick(DeltaTime);
	}//);
}

FString FStateMachineTickFunction::DiagnosticMessage()
{
	if (Target)
	{
		return Target->GetFullName() + TEXT("[Tick]");
	}
	return TEXT("<NULL>[Tick]");

}

FName FStateMachineTickFunction::DiagnosticContext(bool bDetailed)
{
	return FName(TEXT("StateMachineTickFunction"));
}

UStateMachine::UStateMachine(const FObjectInitializer &Initializer)
	: Super(Initializer)
	, ShutdownState(nullptr)
	, CurrentState(nullptr)
	, NextState(nullptr)
{
	AutoTickFunction.bCanEverTick = true;
	AutoTickFunction.Target = this;
}

void UStateMachine::BeginDestroy()
{
	AutoTickFunction.UnRegisterTickFunction();

	Super::BeginDestroy();
}

UWorld* UStateMachine::GetWorld() const
{
	return (!HasAnyFlags(RF_ClassDefaultObject) && GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

bool UStateMachine::IsActive() const
{
	return IsValid(CurrentState) || IsValid(NextState);
}

UObject* UStateMachine::PrepareState(TSubclassOf<UObject> NewStateClass)
{
	return IStateInterface::Execute_ConstructState(GetMutableDefault<UObject>(NewStateClass), this);
}

UObject* UStateMachine::SwitchStateByClass(TSubclassOf<UObject> NewStateClass)
{
	auto NewState = PrepareState(NewStateClass);
	return SwitchState(NewState);
}

UObject* UStateMachine::SwitchState(UObject* NewState)
{
	bool bCurrentStateWasExitedBefore = true;
	if (IsValid(CurrentState))
	{
		auto CurrentStateStatus = IStateInterface::Execute_GetStatus(CurrentState);
		if (CurrentStateStatus != EStateStatus::Exited
			&& CurrentStateStatus != EStateStatus::Inactive)
		{
			UE_LOG(LogStateMachineEx, Verbose, TEXT("State Machine %s exiting state %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());

			bCurrentStateWasExitedBefore = false;
			IStateInterface::Execute_ExitState(CurrentState);

#if !UE_BUILD_SHIPPING
			if (CurrentState != nullptr && IStateInterface::Execute_GetStatus(CurrentState) != EStateStatus::Exited)
			{
				UE_LOG(LogStateMachineExCriticalErrors, Error, TEXT("State Machine %s UState::Exit base function was not called by %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());
			}
#endif
		}
		else
		{
			IStateInterface::Execute_SetStatus(CurrentState, EStateStatus::Inactive);
		}
	}

	CurrentState = nullptr;
	NextState = NewState;

	if (bImmediateStateChange || bCurrentStateWasExitedBefore)
	{
		K2Node_EnterState(NextState);
	}

	return NewState;
}

UObject* UStateMachine::K2Node_EnterState(UObject* NewState)
{
	if (!IsValid(NewState))
		return nullptr;

	CurrentState = NewState;
	NextState = nullptr;

	UE_LOG(LogStateMachineEx, Verbose, TEXT("State Machine %s entering state %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());

	IStateInterface::Execute_EnterState(CurrentState);

#if !UE_BUILD_SHIPPING
	if (CurrentState != nullptr && IStateInterface::Execute_GetStatus(CurrentState) != EStateStatus::Entered)
	{
		UE_LOG(LogStateMachineExCriticalErrors, Error, TEXT("State Machine %s UState::Enter base function was not called by %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());
	}
#endif

	return CurrentState;
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
#if WITH_EDITOR
	if (bIsAutoTickFunction != AutoTickFunction.bCanEverTick)
	{
		if (!AutoTickFunction.bCanEverTick)
		{
			UE_LOG_EX(LogStateMachineExCriticalErrors, Error, TEXT("StateMchine is registered to Tick automatically but was ticked by hand"));
		}
		else
		{
			UE_LOG_EX(LogStateMachineExCriticalErrors, Error, TEXT("StateMchine is Tick by hand but was registered to tick automatically"));
		}
	}
#endif

	while (!IsValid(CurrentState))
	{
		if (!IsValid(NextState))
		{
			// The state machine is shutdown and should be Reset.
			return;
		}

		UE_LOG(LogStateMachineEx, Verbose, TEXT("State Machine %s switched to state %s."), *GetClass()->GetName(), *NextState->GetClass()->GetName());

		CurrentState = NextState;
		NextState = nullptr;
		IStateInterface::Execute_EnterState(CurrentState);

#if !UE_BUILD_SHIPPING
		if (CurrentState != nullptr && IStateInterface::Execute_GetStatus(CurrentState) != EStateStatus::Entered)
		{
			UE_LOG(LogStateMachineExCriticalErrors, Error, TEXT("State Machine %s UState::Enter base function was not called by %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());
		}
#endif
	}

	auto CurrentStateStatus = IStateInterface::Execute_GetStatus(CurrentState);
	if (CurrentStateStatus == EStateStatus::Entered || CurrentStateStatus == EStateStatus::Updated)
	{
#if !UE_BUILD_SHIPPING
		auto PrevCurrentState = CurrentState;
#endif

		IStateInterface::Execute_TickState(CurrentState, DeltaSeconds);

#if !UE_BUILD_SHIPPING
		if (PrevCurrentState == CurrentState)
		{
			if (CurrentState != nullptr && IStateInterface::Execute_GetStatus(CurrentState) != EStateStatus::Updated)
			{
				UE_LOG(LogStateMachineExCriticalErrors, Error, TEXT("State Machine %s UState::Tick base function was not called by %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());
			}
		}
#endif
	}
}

void UStateMachine::Shutdown_Implementation()
{
	if (IsValid(CurrentState))
	{
		if (IsValid(ShutdownState))
		{
			SwitchStateByClass(ShutdownState);
			Tick(GetWorld() ? GetWorld()->GetDeltaSeconds() : 0);
		}
		if (IsValid(CurrentState))
		{
			IStateInterface::Execute_ExitState(CurrentState);

#if !UE_BUILD_SHIPPING
			if (CurrentState != nullptr && IStateInterface::Execute_GetStatus(CurrentState) != EStateStatus::Exited)
			{
				UE_LOG(LogStateMachineExCriticalErrors, Error, TEXT("State Machine %s UState::Exit base function was not called by %s"), *GetClass()->GetName(), *CurrentState->GetClass()->GetName());
			}
#endif
		}
	}

	CurrentState = nullptr;
	NextState = nullptr;
}

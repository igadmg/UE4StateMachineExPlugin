#include "StateMachine/State.h"

#include "StateMachine/StateMachine.h"
#include "InputObject.h"
#include "StateMachineExStatics.h"
#include "UnrealEngineExStatics.h"

#include "Engine/InputDelegateBinding.h"
#include "GameFramework/Pawn.h"

#include "StateMachineEx.final.h"


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

UObject* UState::ConstructState_Implementation(UStateMachine* InStateMachine)
{
	UState* NewState = NewObject<UState>(InStateMachine, GetClass());
	NewState->StateMachine = InStateMachine;

	return NewState;
}

void UState::EnterState_Implementation()
{
	Status = EStateStatus::Entered;

	if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
	{
		InputComponent = UUnrealEngineExStatics::EnableInput(this, XX::GetController<APlayerController>(this), InputComponent);
	}

	UE_LOG(LogStateMachineEx, Verbose, TEXT("Entering state %s State Machine %s"), *GetClass()->GetName(), *StateMachine->GetClass()->GetName());

	OnStateEnter.Broadcast(this);
}

void UState::TickState_Implementation(float DeltaTime)
{
	Status = EStateStatus::Updated;

	OnStateTick.Broadcast(this, DeltaTime);
}

void UState::ExitState_Implementation()
{
	if (auto InternalStateMachine = Valid(UStateMachineExStatics::GuessStateMachineInternal(this)))
	{
		InternalStateMachine->AutoTickFunction.UnRegisterTickFunction();
	}

	OnStateExit.Broadcast(this);

	if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
	{
		UUnrealEngineExStatics::DisableInput(this, XX::GetController<APlayerController>(this), InputComponent);
	}

	// TODO: optimize that
	for (auto* Property : TFieldRange<FObjectProperty>(GetClass()))
	{
		static FName MD_StateMachineCleanup("SM_Cleanup");
		if (Property->HasMetaData(MD_StateMachineCleanup))
		{
			if (auto Object = Property->GetPropertyValue_InContainer(this))
			{
				Property->SetPropertyValue_InContainer(this, nullptr);
				Object->ConditionalBeginDestroy();
			}
		}
	}

	Status = EStateStatus::Exited;

	UE_LOG(LogStateMachineEx, Verbose, TEXT("Exiting state %s State Machine %s"), *GetClass()->GetName(), *StateMachine->GetClass()->GetName());
}

void UState::Restart_Implementation()
{
	Status = EStateStatus::Restarted;
	StateMachine->SwitchStateByClass(GetClass());
}

#include "StateMachine/UserWidgetState.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

#include "StateMachineEx.final.h"



UUserWidgetState::UUserWidgetState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UObject* UUserWidgetState::ConstructState_Implementation(UStateMachine* InStateMachine)
{
	auto Controller = XX::GetController<APlayerController>(InStateMachine);
	if (!IsValid(Controller)) Controller = XX::GetLocalPlayerController(InStateMachine);

	ensure(IsValid(Controller));

	auto Result = ::CreateWidget<UUserWidgetState>(Controller, GetClass());
	Result->StateMachine = InStateMachine;
	return Result;
}

void UUserWidgetState::EnterState_Implementation()
{
	Status = EStateStatus::Entered;

	UE_LOG_EX(LogStateMachineEx, Verbose, TEXT("Entering state %s State Machine %s"), *GetClass()->GetName(), *StateMachine->GetClass()->GetName());

	AddToViewport();

	if (auto PlayerController = XX::GetController<APlayerController>(this))
	{
		switch (InputModeConfiguration.InputMode)
		{
		case EInputMode::UIOnly:
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PlayerController
				, InputModeConfiguration.WidgetToFocus, InputModeConfiguration.MouseLockMode
				, InputModeConfiguration.bShowCursor);
			break;
		case EInputMode::GameAndUI:
			UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController
				, InputModeConfiguration.WidgetToFocus, InputModeConfiguration.MouseLockMode
				, InputModeConfiguration.bShowCursor, InputModeConfiguration.bHideCursorDuringCapture);
			break;
		case EInputMode::GameOnly:
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController
				, !InputModeConfiguration.bShowCursor);
			break;
		}
	}
}

void UUserWidgetState::TickState_Implementation(float DeltaSeconds)
{
	Status = EStateStatus::Updated;
}

void UUserWidgetState::ExitState_Implementation()
{
	RemoveFromParent();

	Status = EStateStatus::Exited;

	UE_LOG_EX(LogStateMachineEx, Verbose, TEXT("Exiting state %s State Machine %s"), *GetClass()->GetName(), *StateMachine->GetClass()->GetName());
}

void UUserWidgetState::Restart_Implementation()
{
	Status = EStateStatus::Restarted;
	StateMachine->SwitchStateByClass(GetClass());
}

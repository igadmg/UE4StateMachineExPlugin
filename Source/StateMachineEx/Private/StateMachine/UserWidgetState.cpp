#include "StateMachine/UserWidgetState.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

#include "StateMachineEx.final.h"



UUserWidgetState::UUserWidgetState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#define UE_LOGFMT_PREFIX(WCO, Format) \
	"{WorldName}> {NetMode}[{NetRole}] {ObjectName} {Function}: " TEXT(Format), \
		("WorldName", ::GetWorldName(this)), \
		("NetMode", ::GetNetMode(this)), \
		("NetRole", ::GetNetRole(this)),\
		("ObjectName", this->GetName()), \
		("Function", UTF8_TO_TCHAR(__FUNCTION__))

#define UE_LOGFMT_PREFIXED(CategoryName, Verbosity, Format, ...) \
	UE_LOGFMT(CategoryName, Verbosity, UE_LOGFMT_PREFIX(this, Format), ##__VA_ARGS__)

#define UE_LOGFMT_PREFIXED_WCO(CategoryName, Verbosity, WorldContextObject, Format, ...) \
	UE_LOGFMT(CategoryName, Verbosity, UE_LOGFMT_PREFIX(WorldContextObject, Format), ##__VA_ARGS__)


UObject* UUserWidgetState::ConstructState_Implementation(UStateMachine* InStateMachine)
{
	auto Controller = XX::GetController<APlayerController>(InStateMachine);
	if (!IsValid(Controller))
	{
		UE_LOGFMT_PREFIXED(LogStateMachineEx, Warning, "Fallback to using Local Player Controller.");
		Controller = XX::GetLocalPlayerController(InStateMachine);
	}

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

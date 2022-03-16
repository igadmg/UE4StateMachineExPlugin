#include "Blueprint/K2Node_State.h"

#include "Blueprint/K2NodeCompilerHelper.h"
#include "Blueprint/K2NodeHelpers.h"
#include "StateMachine/StateMachine.h"
#include "StateMachine/State.h"
#include "StateMachineExStatics.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"

#include "Runtime/Launch/Resources/Version.h"

#include "CoreEx.h"

#define LOCTEXT_NAMESPACE "FStateMachineDeveloperExModule"


FName UK2Node_State::PN_StateClass("StateClass");
FName UK2Node_State::PN_State("State");


UEdGraphPin* UK2Node_State::GetThenPin()
{
	return FindPin(UEdGraphSchema_K2::PN_Then);
}

UEdGraphPin* UK2Node_State::GetStateClassPin()
{
	return FindPin(PN_StateClass);
}

UEdGraphPin* UK2Node_State::GetStatePin()
{
	return FindPin(PN_State);
}

UFunction* UK2Node_State::GetCreateStateObjectFunction() const
{
	return UStateMachineExStatics::StaticClass()->FindFunctionByName(FName("CreateStateObject"));
}

void UK2Node_State::ForEachOutputDelegate(TFunction<void (FMulticastDelegateProperty*)> Predicate)
{
	FK2NodeHelpers::ForEachProperty<FMulticastDelegateProperty>(StateClass, [&Predicate](auto Property) {
		if (auto PropertyClass = Property->GetOwnerClass())
		{
			// Expose only delegates declare in object implementing UStateInterface
			if (!PropertyClass->ImplementsInterface(UStateInterface::StaticClass()))
				return;

			Predicate(Property);
		}
	});
}

FText UK2Node_State::GetTooltipText() const
{
	return LOCTEXT("K2Node_State_Tooltip", "Launches state machine state.");
}

FText UK2Node_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return IsValid(StateClass)
		? FText::Format(LOCTEXT("StateTitleFmt", "State {0}"), FText::FromString(StateClass->GetName()))
		: LOCTEXT("StateTitleUndefined", "State <undefined>");
}

FText UK2Node_State::GetMenuCategory() const
{
	return LOCTEXT("StateMachineCategory", "StateMachine");
}

void UK2Node_State::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	// actions get registered under specific object-keys; the idea is that
	// actions might have to be updated (or deleted) if their object-key is
	// mutated (or removed)... here we use the node's class (so if the node
	// type disappears, then the action should go with it)
	UClass* ActionKey = GetClass();
	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

UObject* UK2Node_State::GetJumpTargetForDoubleClick() const
{
	return IsValid(StateClass) ? StateClass->ClassGeneratedBy : nullptr;
}

void UK2Node_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	if (!bHideThen)
	{
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	}

	if (IsValid(StateClass))
	{
		if (bExposeState)
		{
			CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, StateClass, PN_State);
		}

		ForEachOutputDelegate([this](auto Delegate) {
			FK2NodeHelpers::CreateOutputPins(this, Delegate);
		});

		bool bAllPinsGood = true;
		if (auto Function = GetCreateStateObjectFunction())
		{
			bAllPinsGood = FK2NodeHelpers::CreateInputPins(this, Function);
		}
	}

	UK2Node::AllocateDefaultPins();

	if (UEdGraphPin* StateClassPin = GetStateClassPin())
	{
		StateClassPin->bHidden = true;
		StateClassPin->DefaultObject = StateClass;

		FK2NodeHelpers::CreateOutputPins(this, StateClass);
	}
}

void UK2Node_State::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	UK2Node::ExpandNode(CompilerContext, SourceGraph);

	FK2NodeCompilerHelper Compiler(this, CompilerContext, SourceGraph, GetExecPin(), GetThenPin());

	if (!IsValid(StateClass))
		return;

	// Create a call to factory the state object
	auto CallCreateStateObject = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(EXPAND_FUNCTION_NAME(UStateMachineExStatics, CreateStateObject));
	Compiler.ConnectPins(GetStateClassPin(), CallCreateStateObject->FindPin(PN_StateClass));
	UEdGraphPin* StateObjectPin = CallCreateStateObject->GetReturnValuePin();

	auto ValidateProxyNode = Compiler.SpawnIntermediateNode<UK2Node_IfThenElse>(
		Compiler.SpawnIsValidNode(StateObjectPin)->GetReturnValuePin());

	auto UpCastNode = Compiler.SpawnIntermediateNode<UK2Node_DynamicCast>(StateClass, StateObjectPin);
	StateObjectPin = UpCastNode->GetCastResultPin();

	if (bExposeState)
	{
		Compiler.ConnectPins(StateObjectPin, GetStatePin());
	}

	for (auto SpawnVarPin : Pins)
	{
		if (SpawnVarPin->Direction != EEdGraphPinDirection::EGPD_Input
			&& SpawnVarPin->LinkedTo.Num() == 0)
			continue;

		Compiler.ConnectSetVariable(SpawnVarPin, StateObjectPin);
	}

	ForEachOutputDelegate([this, &Compiler, StateObjectPin](auto Delegate) {
		auto DelegateAndPins = FDelegateAndPins::FindDelegatePins(this, Delegate);
		ensureMsgf(DelegateAndPins.IsValid(), TEXT("Failed to find delegate for function."));

		auto CustomDelegateName = FName(*FString::Printf(TEXT("%s_%p"), *DelegateAndPins.Delegate->GetName(), DelegateAndPins.Delegate));
		auto CustomEventNode = Compiler.SpawnIntermediateEventNode<UK2Node_CustomEvent>(DelegateAndPins.ExecPin, CustomDelegateName, Delegate->SignatureFunction);
		{
			FK2NodeCompilerHelper CustomEventCompiler(Compiler, Compiler.GetThenPin(CustomEventNode), DelegateAndPins.ExecPin);
			auto GuessCurrentState = CustomEventCompiler.SpawnIntermediateNode<UK2Node_CallFunction>(EXPAND_FUNCTION_NAME(UStateMachineExStatics, GuessCurrentStateInternal));
			auto CurrentState = CustomEventCompiler.SpawnIntermediateNode<UK2Node_DynamicCast>(StateClass, GuessCurrentState->GetReturnValuePin());
			CustomEventCompiler.SpawnIntermediateNode<UK2Node_CallFunction>(CurrentState->GetCastResultPin(), StateClass, GET_FUNCTION_NAME_CHECKED(IStateInterface, ExitState));

			for (auto OutputPin : DelegateAndPins.ParameterPins)
			{
				if (auto EventDataPin = CustomEventNode->FindPin(OutputPin->PinName))
				{
					CustomEventCompiler.ConnectPins(EventDataPin, OutputPin);
				}
			}
		}

		// Bind Custom Event to State Object delegate.
		auto AddDelegateNode = Compiler.SpawnIntermediateNode<UK2Node_AddDelegate>(DelegateAndPins.Delegate, StateObjectPin, CustomEventNode->FindPin(UK2Node_Event::DelegateOutputName));
	});

	auto CallGetParentStateMachine = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(StateObjectPin, StateClass, GET_FUNCTION_NAME_CHECKED(IStateInterface, GetStateMachine));
	auto ParentStateMachinePin = CallGetParentStateMachine->GetReturnValuePin();
	auto CallSwitchState = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(ParentStateMachinePin, EXPAND_FUNCTION_NAME(UStateMachine, SwitchState));
	Compiler.ConnectPins(StateObjectPin, CallSwitchState->FindPin(TEXT("NewState")));

	if (bExposeState)
	{
		Compiler.ConnectPins(StateObjectPin, GetStatePin());
	}
}

bool UK2Node_State::HasExternalDependencies(TArray<class UStruct*>* OptionalOutput) const
{
	const bool bResult = IsValid(StateClass);
	if (bResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(StateClass);
	}

	return Super::HasExternalDependencies(OptionalOutput) || bResult;
}

#if WITH_EDITOR
void UK2Node_State::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if_PropertyChanged(StateClass)
	{
		if (auto StateClassPin = GetStateClassPin())
		{
			StateClassPin->DefaultObject = StateClass;
		}
	}

	ReconstructNode();
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

#include "Blueprint/K2Node_State.h"
#include "Blueprint/K2NodeHelpers.h"
#include "StateMachine/StateMachine.h"
#include "StateMachine/State.h"
#include "StateMachineExStatics.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CallArrayFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_EnumLiteral.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_VariableGet.h"

#include "Runtime/Launch/Resources/Version.h"

#include "CoreEx.h"

#define LOCTEXT_NAMESPACE "FStateMachineDeveloperExModule"



bool UK2Node_State::FStateHelper::HandleDelegateImplementation(
	FMulticastDelegateProperty* CurrentProperty, const TArray<FStateHelper::FOutputPinAndLocalVariable>& VariableOutputs,
	UEdGraphPin* ProxyObjectPin, UClass* ProxyClass, UEdGraphPin*& InOutLastThenPin,
	FK2NodeCompilerHelper& Compiler)
{
	bool bIsErrorFree = true;
	const UEdGraphSchema_K2* Schema = Compiler.CompilerContext.GetSchema();
	check(CurrentProperty && ProxyObjectPin && InOutLastThenPin && Schema);

	UEdGraphPin* PinForCurrentDelegateProperty = Compiler.Node->FindPin(CurrentProperty->GetFName());
	if (!PinForCurrentDelegateProperty || (UEdGraphSchema_K2::PC_Exec != PinForCurrentDelegateProperty->PinType.PinCategory))
	{
		//FText ErrorMessage = FText::Format(LOCTEXT("WrongDelegateProperty", "BaseAsyncTask: Cannot find execution pin for delegate "), FText::FromString(CurrentProperty->GetName()));
		//CompilerContext.MessageLog.Error(*ErrorMessage.ToString(), CurrentNode);
		return true;
	}

	UK2Node_CustomEvent* CurrentCENode = Compiler.SpawnIntermediateEventNode<UK2Node_CustomEvent>(PinForCurrentDelegateProperty
		, FName(*FString::Printf(TEXT("%s_%s"), *CurrentProperty->GetName(), *Compiler.CompilerContext.GetGuid(Compiler.Node))));
	{
		UK2Node_AddDelegate* AddDelegateNode = Compiler.SpawnIntermediateNode<UK2Node_AddDelegate>(CurrentProperty, ProxyObjectPin);
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastThenPin, AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute));
		InOutLastThenPin = AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

		bIsErrorFree &= FBaseAsyncTaskHelper::CreateDelegateForNewFunction(AddDelegateNode->GetDelegatePin(), CurrentCENode->GetFunctionName(), Cast<UK2Node>(Compiler.Node), Compiler.SourceGraph, Compiler.CompilerContext);
		bIsErrorFree &= FBaseAsyncTaskHelper::CopyEventSignature(CurrentCENode, AddDelegateNode->GetDelegateSignature(), Schema);
	}

	UEdGraphPin* LastActivatedNodeThen = CurrentCENode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
	for (const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& OutputPair : VariableOutputs) // CREATE CHAIN OF ASSIGMENTS
	{
		UEdGraphPin* PinWithData = CurrentCENode->FindPin(OutputPair.OutputPin->PinName);
		if (PinWithData == nullptr)
		{
			/*FText ErrorMessage = FText::Format(LOCTEXT("MissingDataPin", "ICE: Pin @@ was expecting a data output pin named {0} on @@ (each delegate must have the same signature)"), FText::FromString(OutputPair.OutputPin->PinName));
			CompilerContext.MessageLog.Error(*ErrorMessage.ToString(), OutputPair.OutputPin, CurrentCENode);
			return false;*/
			continue;
		}

		auto AssignNode = Compiler.SpawnIntermediateNode<UK2Node_AssignmentStatement>();
		bIsErrorFree &= Schema->TryCreateConnection(LastActivatedNodeThen, AssignNode->GetExecPin());
		bIsErrorFree &= Schema->TryCreateConnection(OutputPair.TempVar->GetVariablePin(), AssignNode->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetVariablePin());
		bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), PinWithData);
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetValuePin());

		LastActivatedNodeThen = AssignNode->GetThenPin();
	}

	auto CallExitStateNode = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(ProxyClass, GET_FUNCTION_NAME_CHECKED(IStateInterface, ExitState));

	UEdGraphPin* SelfInputPin = CallExitStateNode->FindPinChecked(TEXT("self"));
	bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, SelfInputPin);

	bIsErrorFree &= Schema->TryCreateConnection(LastActivatedNodeThen, CallExitStateNode->GetExecPin());
	LastActivatedNodeThen = CallExitStateNode->GetThenPin();

	bIsErrorFree &= Compiler.MovePinLinksToIntermediate(PinForCurrentDelegateProperty, LastActivatedNodeThen);
	return bIsErrorFree;
}

UK2Node_State::UK2Node_State(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UStateMachineExStatics, CreateStateObject);
	ProxyFactoryClass = UStateMachineExStatics::StaticClass();

	ProxyClass = UState::StaticClass();
}

UEdGraphPin* UK2Node_State::GetStateClassPin()
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->GetName() == TEXT("StateClass"))
		{
			return Pin;
		}
	}

	return nullptr;
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

UObject* UK2Node_State::GetJumpTargetForDoubleClick() const
{
	return IsValid(StateClass) ? StateClass->ClassGeneratedBy : nullptr;
}

void UK2Node_State::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
	CreatePin(EGPD_Input, K2Schema->PC_Exec, FName(), nullptr, K2Schema->PN_Execute);
#else
	CreatePin(EGPD_Input, K2Schema->PC_Exec, FString(), nullptr, K2Schema->PN_Execute);
#endif

	bool bExposeProxy = false;
	bool bHideThen = true;
	for (const UStruct* TestStruct = ProxyClass; TestStruct; TestStruct = TestStruct->GetSuperStruct())
	{
		bExposeProxy |= TestStruct->HasMetaData(TEXT("ExposedAsyncProxy"));
	}

	if (!bHideThen)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		CreatePin(EGPD_Output, K2Schema->PC_Exec, FName(), nullptr, K2Schema->PN_Then);
#else
		CreatePin(EGPD_Output, K2Schema->PC_Exec, FString(), nullptr, K2Schema->PN_Then);
#endif
	}

	if (bExposeProxy)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		CreatePin(EGPD_Output, K2Schema->PC_Object, FName(), ProxyClass, FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
#else
		CreatePin(EGPD_Output, K2Schema->PC_Object, FString(), ProxyClass, FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
#endif
	}

	for (TFieldIterator<FProperty> PropertyIt(ProxyClass); PropertyIt; ++PropertyIt)
	{
		if (auto AsMulticastDelegateProperty = Valid<FMulticastDelegateProperty>(*PropertyIt))
		{
			if (auto PropertyClass = AsMulticastDelegateProperty->GetOwnerClass())
			{
				if (!PropertyClass->ImplementsInterface(UStateInterface::StaticClass()))
					continue;

				FK2NodeHelpers::CreateOutputPins(this, AsMulticastDelegateProperty);
			}
		}
	}

	bool bAllPinsGood = true;
	UFunction* Function = ProxyFactoryClass ? ProxyFactoryClass->FindFunctionByName(ProxyFactoryFunctionName) : nullptr;
	if (Function)
	{
		bAllPinsGood = FK2NodeHelpers::CreateInputPins(this, Function);
	}

	UK2Node::AllocateDefaultPins();

	UEdGraphPin* StateClassPin = GetStateClassPin();
	check(StateClassPin);

	StateClassPin->bHidden = true;
	StateClassPin->DefaultObject = StateClass;

	FK2NodeHelpers::CreateOutputPins(this, StateClass);
}

void UK2Node_State::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	auto Schema = CompilerContext.GetSchema();
	bool bIsErrorFree = true;

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
	static FName ObjectParamName = FName(TEXT("Object"));
	static FName ValueParamName = FName(TEXT("Value"));
	static FName PropertyNameParamName = FName(TEXT("PropertyName"));
#else
	static FString ObjectParamName = FString(TEXT("Object"));
	static FString ValueParamName = FString(TEXT("Value"));
	static FString PropertyNameParamName = FString(TEXT("PropertyName"));
	static FString PropertyNameParamName = FString(TEXT("PropertyName"));
#endif

	UK2Node::ExpandNode(CompilerContext, SourceGraph);

	FK2NodeCompilerHelper Compiler(this, CompilerContext, SourceGraph);

	// Create a call to factory the proxy object
	auto CallCreateProxyObjectNode = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(ProxyFactoryClass, ProxyFactoryFunctionName);
	if (CallCreateProxyObjectNode->GetTargetFunction() == nullptr)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20
		const FText ClassName = ProxyFactoryClass ? FText::FromString(ProxyFactoryClass->GetName()) : LOCTEXT("MissingClassString", "Unknown Class");
		const FString FormattedMessage = FText::Format(
			LOCTEXT("AsyncTaskErrorFmt", "State: Missing function {0} from class {1} for async task @@"),
			FText::FromString(ProxyFactoryFunctionName.GetPlainNameString()),
			ClassName
		).ToString();
#else
		const FString ClassName = ProxyFactoryClass ? ProxyFactoryClass->GetName() : LOCTEXT("MissingClassString", "Unknown Class").ToString();
		const FString RawMessage = LOCTEXT("AsyncTaskError", "State: Missing function %s from class %s for async task @@").ToString();
		const FString FormattedMessage = FString::Printf(*RawMessage, *ProxyFactoryFunctionName.GetPlainNameString(), *ClassName);
#endif

		CompilerContext.MessageLog.Error(*FormattedMessage, this);
		return;
	}

	bIsErrorFree &= Compiler.MovePinLinksToIntermediate(this, Schema->PN_Execute, CallCreateProxyObjectNode, Schema->PN_Execute);

	for (auto CurrentPin : Pins)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input))
#else
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input, Schema))
#endif
		{
			bIsErrorFree &= Compiler.MovePinLinksToIntermediate(CurrentPin, CallCreateProxyObjectNode->FindPin(CurrentPin->PinName), true);
		}
	}

	UEdGraphPin* ProxyObjectPin = CallCreateProxyObjectNode->GetReturnValuePin();
	UEdGraphPin* OutputAsyncTaskProxy = FindPin(FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
	check(ProxyObjectPin);

	bIsErrorFree &= Compiler.MovePinLinksToIntermediate(OutputAsyncTaskProxy, ProxyObjectPin, true);

	// GATHER OUTPUT PARAMETERS AND PAIR THEM WITH LOCAL VARIABLES
	TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable> VariableOutputs;
	for (auto CurrentPin : Pins)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		if ((OutputAsyncTaskProxy != CurrentPin) && FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Output))
#else
		if ((OutputAsyncTaskProxy != CurrentPin) && FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Output, Schema))
#endif
		{
			auto TempVarOutput = Compiler.SpawnInternalVariable(CurrentPin->PinType);
			bIsErrorFree &= Compiler.MovePinLinksToIntermediate(CurrentPin, TempVarOutput->GetVariablePin());
			VariableOutputs.Add(FBaseAsyncTaskHelper::FOutputPinAndLocalVariable(CurrentPin, TempVarOutput));
		}
	}

	// FOR EACH DELEGATE DEFINE EVENT, CONNECT IT TO DELEGATE AND IMPLEMENT A CHAIN OF ASSIGMENTS
	UEdGraphPin* LastThenPin = CallCreateProxyObjectNode->FindPinChecked(Schema->PN_Then);

	auto ValidateProxyNode = Compiler.SpawnIntermediateNode<UK2Node_IfThenElse>(
		Compiler.SpawnIsValidNode(ProxyObjectPin)->GetReturnValuePin());

	bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ValidateProxyNode->GetExecPin());
	LastThenPin = ValidateProxyNode->GetThenPin();

	auto UpCastNode = Compiler.SpawnIntermediateNode<UK2Node_DynamicCast>(ProxyClass, ProxyObjectPin);
	ProxyObjectPin = UpCastNode->GetCastResultPin();

	bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, UpCastNode->GetExecPin());
	LastThenPin = UpCastNode->GetValidCastPin();

	// Create 'set var by name' nodes and hook them up
	for (auto SpawnVarPin : Pins)
	{
		// Only create 'set param by name' node if this pin is linked to something
		if (SpawnVarPin->LinkedTo.Num() > 0)
		{
			bIsErrorFree &= Compiler.CreateSetParamByNameNodes(ProxyObjectPin, SpawnVarPin, LastThenPin);
		}
	}

	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass, EFieldIteratorFlags::IncludeSuper); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		bIsErrorFree &= FStateHelper::HandleDelegateImplementation(*PropertyIt, VariableOutputs, ProxyObjectPin, ProxyClass, LastThenPin, Compiler);
	}

	auto CallGetParentStateMachine = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(ProxyClass, GET_FUNCTION_NAME_CHECKED(IStateInterface, GetParentStateMachine));
	bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, CallGetParentStateMachine->FindPinChecked(TEXT("self")));
	//bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, CallGetParentStateMachine->GetExecPin());
	//LastThenPin = CallGetParentStateMachine->GetThenPin();

	auto CallSwitchState = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(UStateMachine::StaticClass(), GET_FUNCTION_NAME_CHECKED(UStateMachine, SwitchState));

	UEdGraphPin* NewStateInputPin = CallSwitchState->FindPinChecked(TEXT("NewState"));
	bIsErrorFree &= Schema->TryCreateConnection(CallGetParentStateMachine->GetReturnValuePin(), CallSwitchState->FindPinChecked(TEXT("self")));
	bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, NewStateInputPin);
	bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, CallSwitchState->GetExecPin());
	LastThenPin = CallSwitchState->GetThenPin();

	if (CallCreateProxyObjectNode->FindPinChecked(Schema->PN_Then) == LastThenPin)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("MissingDelegateProperties", "BaseAsyncTask: Proxy has no delegates defined. @@").ToString(), this);
		return;
	}

	// Create a call to activate the proxy object if necessary
	if (ProxyActivateFunctionName != NAME_None)
	{
		auto CallActivateProxyObjectNode = Compiler.SpawnIntermediateNode<UK2Node_CallFunction>(ProxyClass, ProxyActivateFunctionName);

		// Hook up the self connection
		UEdGraphPin* ActivateCallSelfPin = Schema->FindSelfPin(*CallActivateProxyObjectNode, EGPD_Input);
		check(ActivateCallSelfPin);

		bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, ActivateCallSelfPin);

		// Hook the activate node up in the exec chain
		UEdGraphPin* ActivateExecPin = CallActivateProxyObjectNode->FindPinChecked(Schema->PN_Execute);
		UEdGraphPin* ActivateThenPin = CallActivateProxyObjectNode->FindPinChecked(Schema->PN_Then);

		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ActivateExecPin);

		LastThenPin = ActivateThenPin;
	}

	// Move the connections from the original node then pin to the last internal then pin
	//bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(Schema->PN_Then), *LastThenPin).CanSafeConnect();
	bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*LastThenPin, *ValidateProxyNode->GetElsePin()).CanSafeConnect();

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "BaseAsyncTask: Internal connection error. @@").ToString(), this);
	}

	// Make sure we caught everything
	BreakAllNodeLinks();
}

#if WITH_EDITOR
void UK2Node_State::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static FName NAME_StateClass(TEXT("StateClass"));

	// The brush builder that created this volume has changed. Notify listeners
	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.MemberProperty->GetFName() == NAME_StateClass)
	{
		UEdGraphPin* StateClassPin = GetStateClassPin();
		check(StateClassPin);

		StateClassPin->DefaultObject = StateClass;
		ProxyClass = StateClass;

		ReconstructNode();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

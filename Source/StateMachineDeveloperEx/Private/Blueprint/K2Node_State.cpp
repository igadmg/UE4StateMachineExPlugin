#include "Blueprint/K2Node_State.h"
#include "Blueprint/CreateStateAsyncTask.h"
#include "StateMachine/State.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CallArrayFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"

#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "FStateMachineDeveloperExModule"



UK2Node_State::UK2Node_State(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UCreateStateAsyncTask, CreateStateObject);
	ProxyFactoryClass = UCreateStateAsyncTask::StaticClass();

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
	return LOCTEXT("K2Node_LeaderboardFlush_Tooltip", "Flushes leaderboards for a session");
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
	return IsValid(ProxyClass) ? ProxyClass->ClassGeneratedBy : nullptr;
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
	bool bHideThen = false;
	for (const UStruct* TestStruct = ProxyClass; TestStruct; TestStruct = TestStruct->GetSuperStruct())
	{
		bExposeProxy |= TestStruct->HasMetaData(TEXT("ExposedAsyncProxy"));
		bHideThen |= TestStruct->HasMetaData(TEXT("HideThen"));
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
		if (auto* Property = CastField<FMulticastDelegateProperty>(*PropertyIt))
		{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
			CreatePin(EGPD_Output, K2Schema->PC_Exec, FName(), nullptr, *Property->GetName());
#else
			CreatePin(EGPD_Output, K2Schema->PC_Exec, FString(), nullptr, *Property->GetName());
#endif
			UFunction* DelegateSignatureFunction = Property->SignatureFunction;
			if (IsValid(DelegateSignatureFunction))
			{
				for (TFieldIterator<FProperty> PropIt(DelegateSignatureFunction); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
				{
					auto* Param = *PropIt;
					const bool bIsFunctionInput = !Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm);
					if (bIsFunctionInput)
					{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
						UEdGraphPin* Pin = CreatePin(EGPD_Output, FName(), FName(), nullptr, Param->GetFName());
#else
						UEdGraphPin* Pin = CreatePin(EGPD_Output, FString(), FString(), nullptr, Param->GetName());
#endif
						K2Schema->ConvertPropertyToPinType(Param, /*out*/ Pin->PinType);
					}
				}
			}
		}
	}

	bool bAllPinsGood = true;
	UFunction* Function = ProxyFactoryClass ? ProxyFactoryClass->FindFunctionByName(ProxyFactoryFunctionName) : nullptr;
	if (Function)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		TSet<FName> PinsToHide;
		FBlueprintEditorUtils::GetHiddenPinsForFunction(GetGraph(), Function, PinsToHide);
#else
		TSet<FString> PinsToHide;
		FBlueprintEditorUtils::GetHiddenPinsForFunction(GetGraph(), Function, PinsToHide);
#endif
		for (TFieldIterator<FProperty> PropIt(Function); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			auto* Param = *PropIt;
			const bool bIsFunctionInput = !Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm);
			if (!bIsFunctionInput)
			{
				// skip function output, it's internal node data
				continue;
			}

			const bool bIsRefParam = Param->HasAnyPropertyFlags(CPF_ReferenceParm) && bIsFunctionInput;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
			FCreatePinParams PinParams;
			PinParams.bIsReference = bIsRefParam;
			UEdGraphPin* Pin = CreatePin(EGPD_Input, FName(), FName(), nullptr, Param->GetFName(), PinParams);
#else
			UEdGraphPin* Pin = CreatePin(EGPD_Input, FString(), FString(), nullptr, Param->GetName(), EPinContainerType::None, bIsRefParam);
#endif
			const bool bPinGood = (Pin != NULL) && K2Schema->ConvertPropertyToPinType(Param, /*out*/ Pin->PinType);

			if (bPinGood)
			{
				//Flag pin as read only for const reference property
				Pin->bDefaultValueIsIgnored = Param->HasAllPropertyFlags(CPF_ConstParm | CPF_ReferenceParm) && (!Function->HasMetaData(FBlueprintMetadata::MD_AutoCreateRefTerm) || Pin->PinType.IsContainer());

				const bool bAdvancedPin = Param->HasAllPropertyFlags(CPF_AdvancedDisplay);
				Pin->bAdvancedView = bAdvancedPin;
				if (bAdvancedPin && (ENodeAdvancedPins::NoPins == AdvancedPinDisplay))
				{
					AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
				}

				FString ParamValue;
				if (K2Schema->FindFunctionParameterDefaultValue(Function, Param, ParamValue))
				{
					K2Schema->SetPinAutogeneratedDefaultValue(Pin, ParamValue);
				}
				else
				{
					K2Schema->SetPinAutogeneratedDefaultValueBasedOnType(Pin);
				}

				if (PinsToHide.Contains(Pin->PinName))
				{
					Pin->bHidden = true;
				}
			}

			bAllPinsGood = bAllPinsGood && bPinGood;
		}
	}

	UK2Node::AllocateDefaultPins();

	UEdGraphPin* StateClassPin = GetStateClassPin();
	check(StateClassPin);

	StateClassPin->bHidden = true;
	StateClassPin->DefaultObject = StateClass;

	for (TFieldIterator<FProperty> PropertyIt(StateClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		auto* Property = *PropertyIt;
		//UClass* PropertyClass = CastChecked<UClass>(Property->GetOuter());
		const bool bIsDelegate = Property->IsA(FMulticastDelegateProperty::StaticClass());
		const bool bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn(Property);
		const bool bIsSettableExternally = !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance);

		if (bIsExposedToSpawn &&
			!Property->HasAnyPropertyFlags(CPF_Parm) &&
			bIsSettableExternally &&
			Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
			!bIsDelegate)
		{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
			UEdGraphPin* Pin = CreatePin(EGPD_Input, FName(), FName(), nullptr, Property->GetFName());
#else
			UEdGraphPin* Pin = CreatePin(EGPD_Input, FString(), FString(), nullptr, Property->GetName());
#endif
			const bool bPinGood = (Pin != nullptr) && K2Schema->ConvertPropertyToPinType(Property, /*out*/ Pin->PinType);

			// Copy tooltip from the property.
			if (Pin != nullptr)
			{
				K2Schema->ConstructBasicPinTooltip(*Pin, Property->GetToolTipText(), Pin->PinToolTip);
			}
		}
	}
}

void UK2Node_State::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	static FString ObjectParamName = FString(TEXT("Object"));
	static FString ValueParamName = FString(TEXT("Value"));
	static FString PropertyNameParamName = FString(TEXT("PropertyName"));

	UK2Node::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(SourceGraph && Schema);
	bool bIsErrorFree = true;

	// Create a call to factory the proxy object
	UK2Node_CallFunction* const CallCreateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallCreateProxyObjectNode->FunctionReference.SetExternalMember(ProxyFactoryFunctionName, ProxyFactoryClass);
	CallCreateProxyObjectNode->AllocateDefaultPins();
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

	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(Schema->PN_Execute), *CallCreateProxyObjectNode->FindPinChecked(Schema->PN_Execute)).CanSafeConnect();

	for (auto CurrentPin : Pins)
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input))
#else
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input, Schema))
#endif
		{
			UEdGraphPin* DestPin = CallCreateProxyObjectNode->FindPin(CurrentPin->PinName); // match function inputs, to pass data to function from CallFunction node
			if (DestPin != nullptr)
			{
				bIsErrorFree &= DestPin && CompilerContext.MovePinLinksToIntermediate(*CurrentPin, *DestPin).CanSafeConnect();
			}
		}
	}

	UEdGraphPin* ProxyObjectPin = CallCreateProxyObjectNode->GetReturnValuePin();
	check(ProxyObjectPin);
	UEdGraphPin* OutputAsyncTaskProxy = FindPin(FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
	bIsErrorFree &= !OutputAsyncTaskProxy || CompilerContext.MovePinLinksToIntermediate(*OutputAsyncTaskProxy, *ProxyObjectPin).CanSafeConnect();

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
			const FEdGraphPinType& PinType = CurrentPin->PinType;
			UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
				this, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(), PinType.ContainerType, PinType.PinValueType);
			bIsErrorFree &= TempVarOutput->GetVariablePin() && CompilerContext.MovePinLinksToIntermediate(*CurrentPin, *TempVarOutput->GetVariablePin()).CanSafeConnect();
			VariableOutputs.Add(FBaseAsyncTaskHelper::FOutputPinAndLocalVariable(CurrentPin, TempVarOutput));
		}
	}

	// FOR EACH DELEGATE DEFINE EVENT, CONNECT IT TO DELEGATE AND IMPLEMENT A CHAIN OF ASSIGMENTS
	UEdGraphPin* LastThenPin = CallCreateProxyObjectNode->FindPinChecked(Schema->PN_Then);

	UK2Node_CallFunction* IsValidFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	const FName IsValidFuncName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid);
	IsValidFuncNode->FunctionReference.SetExternalMember(IsValidFuncName, UKismetSystemLibrary::StaticClass());
	IsValidFuncNode->AllocateDefaultPins();
	UEdGraphPin* IsValidInputPin = IsValidFuncNode->FindPinChecked(TEXT("Object"));

	bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, IsValidInputPin);

	UK2Node_IfThenElse* ValidateProxyNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	ValidateProxyNode->AllocateDefaultPins();
	bIsErrorFree &= Schema->TryCreateConnection(IsValidFuncNode->GetReturnValuePin(), ValidateProxyNode->GetConditionPin());

	bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ValidateProxyNode->GetExecPin());
	LastThenPin = ValidateProxyNode->GetThenPin();

	UK2Node_DynamicCast* UpCastNode = CompilerContext.SpawnIntermediateNode<UK2Node_DynamicCast>(this, SourceGraph);
	UpCastNode->TargetType = ProxyClass;
	UpCastNode->AllocateDefaultPins();
	bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, UpCastNode->GetCastSourcePin());
	ProxyObjectPin = UpCastNode->GetCastResultPin();

	bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, UpCastNode->GetExecPin());
	LastThenPin = UpCastNode->GetValidCastPin();

	// Create 'set var by name' nodes and hook them up
	for (int32 PinIdx = 0; PinIdx < Pins.Num(); PinIdx++)
	{
		// Only create 'set param by name' node if this pin is linked to something
		UEdGraphPin* SpawnVarPin = Pins[PinIdx];
		if (SpawnVarPin->LinkedTo.Num() > 0)
		{
			UFunction* SetByNameFunction = Schema->FindSetVariableByNameFunction(SpawnVarPin->PinType);
			if (SetByNameFunction)
			{
				UK2Node_CallFunction* SetVarNode = NULL;
				if (SpawnVarPin->PinType.IsArray())
				{
					SetVarNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallArrayFunction>(this, SourceGraph);
				}
				else
				{
					SetVarNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
				}
				SetVarNode->SetFromFunction(SetByNameFunction);
				SetVarNode->AllocateDefaultPins();

				// Connect this node into the exec chain
				bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, SetVarNode->GetExecPin());

				// Connect the new actor to the 'object' pin
				bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, SetVarNode->FindPinChecked(ObjectParamName));

				// Fill in literal for 'property name' pin - name of pin is property name
				UEdGraphPin* PropertyNamePin = SetVarNode->FindPinChecked(PropertyNameParamName);
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
				PropertyNamePin->DefaultValue = SpawnVarPin->PinName.ToString();
#else
				PropertyNamePin->DefaultValue = SpawnVarPin->PinName;
#endif

				// Move connection from the variable pin on the spawn node to the 'value' pin
				UEdGraphPin* ValuePin = SetVarNode->FindPinChecked(ValueParamName);
				CompilerContext.MovePinLinksToIntermediate(*SpawnVarPin, *ValuePin);
				if (SpawnVarPin->PinType.IsArray())
				{
					SetVarNode->PinConnectionListChanged(ValuePin);
				}

				// Update 'last node in sequence' var
				LastThenPin = SetVarNode->GetThenPin();
			}
		}
	}

	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass, EFieldIteratorFlags::IncludeSuper); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		bIsErrorFree &= FBaseAsyncTaskHelper::HandleDelegateImplementation(*PropertyIt, VariableOutputs, ProxyObjectPin, LastThenPin, this, SourceGraph, CompilerContext);
	}

	if (CallCreateProxyObjectNode->FindPinChecked(Schema->PN_Then) == LastThenPin)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("MissingDelegateProperties", "BaseAsyncTask: Proxy has no delegates defined. @@").ToString(), this);
		return;
	}

	// Create a call to activate the proxy object if necessary
	if (ProxyActivateFunctionName != NAME_None)
	{
		UK2Node_CallFunction* const CallActivateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallActivateProxyObjectNode->FunctionReference.SetExternalMember(ProxyActivateFunctionName, ProxyClass);
		CallActivateProxyObjectNode->AllocateDefaultPins();

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
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(Schema->PN_Then), *LastThenPin).CanSafeConnect();
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

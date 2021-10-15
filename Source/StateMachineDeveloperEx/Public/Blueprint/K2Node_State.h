#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "K2Node_BaseAsyncTask.h"
#include "K2Node_State.generated.h"



UCLASS()
class STATEMACHINEDEVELOPEREX_API UK2Node_State : public UK2Node_BaseAsyncTask
{
	GENERATED_BODY()


	struct STATEMACHINEDEVELOPEREX_API FStateHelper : public FBaseAsyncTaskHelper
	{
		static bool HandleDelegateImplementation(
			FMulticastDelegateProperty* CurrentProperty, const TArray<FStateHelper::FOutputPinAndLocalVariable>& VariableOutputs,
			UEdGraphPin* ProxyObjectPin, UClass* ProxyClass, UEdGraphPin*& InOutLastThenPin,
			class FK2NodeCompilerHelper& Compiler);
	};


public:
	UPROPERTY(Category = "State", EditAnywhere, BlueprintReadOnly, meta = (MustImplement = "StateInterface"))
	TSubclassOf<class UObject> StateClass;



public:
	UK2Node_State(const FObjectInitializer& ObjectInitializer);

	UEdGraphPin* GetStateClassPin();


	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;

	virtual bool ShouldShowNodeProperties() const override { return true; }

	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;



#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};

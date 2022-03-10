#pragma once

#include "K2Node.h"

#include "K2Node_State.generated.h"



UCLASS()
class STATEMACHINEDEVELOPEREX_API UK2Node_State : public UK2Node
{
	GENERATED_BODY()


public:
	static FName PN_StateClass;
	static FName PN_State;


public:
	UPROPERTY(Category = "State", EditAnywhere, BlueprintReadOnly, meta = (MustImplement = "StateInterface"))
	TSubclassOf<class UObject> StateClass;

	UPROPERTY(Category = "State", EditAnywhere, BlueprintReadOnly)
	bool bExposeState = false;

	UPROPERTY(Category = "State", EditAnywhere, BlueprintReadOnly)
	bool bHideThen = true;


public:
	class UEdGraphPin* GetThenPin();

	class UEdGraphPin* GetStateClassPin();
	class UEdGraphPin* GetStatePin();


protected:
	class UFunction* GetCreateStateObjectFunction() const;

	void ForEachOutputDelegate(TFunction<void (class FMulticastDelegateProperty*)> Predicate);


protected:
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	virtual void GetMenuActions(class FBlueprintActionDatabaseRegistrar& ActionRegistrar) const;

	virtual UObject* GetJumpTargetForDoubleClick() const override;

	virtual bool ShouldShowNodeProperties() const override { return true; }

	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};

#pragma once

#include "Blueprint/UserWidget.h"
#include "StateMachine/StateInterface.h"
#include "UnrealEngineExTypes.h"

#include "UserWidgetState.generated.h"


UCLASS()
class STATEMACHINEEX_API UUserWidgetState
	: public UUserWidget
	, public IStateInterface
{
	GENERATED_BODY()


public:
	UPROPERTY(Category = "State", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 StateId;

	UPROPERTY(Category = "State", VisibleInstanceOnly, BlueprintReadOnly)
	EStateStatus Status = EStateStatus::Inactive;

	UPROPERTY(Category = "State", VisibleInstanceOnly, BlueprintReadOnly, Transient)
	class UStateMachine* StateMachine;

	UPROPERTY(Category = "Widget", EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FInputModeConfiguration InputModeConfiguration;


public: /// IStateInterface
	virtual UObject* ConstructState_Implementation(class UStateMachine *InStateMachine) override;
	virtual class UStateMachine* GetStateMachine_Implementation() const override { return StateMachine; }
	virtual uint8 GetStateId_Implementation() const override { return StateId; }
	virtual EStateStatus GetStatus_Implementation() const override { return Status; }
	virtual void SetStatus_Implementation(EStateStatus InStatus) override { Status = InStatus; }
	virtual void EnterState_Implementation() override;
	virtual void TickState_Implementation(float DeltaSeconds) override;
	virtual void ExitState_Implementation() override;
	virtual void Restart_Implementation() override;


public:
	UUserWidgetState(const FObjectInitializer& ObjectInitializer);
};


#pragma once

#include "StateMachine/State.h"

#include "StateMachineState.generated.h"


UCLASS()
class STATEMACHINEEX_API UStateMachineState : public UState
{
	GENERATED_BODY()


public:
	UPROPERTY(Category = "State", BlueprintReadOnly)
	class UStateMachine* InternalStateMachine;


public:
	UStateMachineState(const FObjectInitializer& ObjectInitializer);


protected:
	virtual void EnterState_Implementation() override;
	virtual void TickState_Implementation(float DeltaSeconds) override;
	virtual void ExitState_Implementation() override;
};


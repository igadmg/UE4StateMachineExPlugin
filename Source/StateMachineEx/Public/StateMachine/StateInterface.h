#pragma once

#include "UObject/Interface.h"

#include "StateInterface.generated.h"


UENUM(BlueprintType)
enum class EStateStatus : uint8
{
	Inactive,
	Restarted,
	Entered,
	Updated,
	Exited,
	Paused,
};

UINTERFACE(Blueprintable)
class STATEMACHINEEX_API UStateInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 State interface.
 Implementing state interface class can be used as a state in state machine.
 States have Enter/Exit functions executed when state machine enter or exit state
 and Update function which is called every time State Machie is updated.

 Sates are special deferred nodes in blueprint which can be connected any way blueprint nodes are connected.
 States in Bluepinr Graph automaticaly use State Machine of their owning bluepinr class.

 States are encapsulated logic blocks. They don't need to nowanything about their state machine.
 They just execute some code on Enter/Exit or Update. If state wnat to finish itself it provide delegate events.
 These events appear as exetuion pins on the Blueprin Graph and can be connected to other states or some blueprint code.
 If state whants to exit itself someway it just call that delegate event.
*/

class STATEMACHINEEX_API IStateInterface
{
	GENERATED_BODY()


public:
	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent)
	UObject* ConstructState(class UStateMachine *StateMachine);

	UFUNCTION(Category = "State Machine: State", BlueprintCallable, BlueprintNativeEvent)
	class UStateMachine* GetParentStateMachine() const;

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent)
	uint8 GetStateId() const;

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent)
	EStateStatus GetStatus() const;

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent)
	void SetStatus(EStateStatus Status);

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void EnterState();

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void TickState(float DeltaSeconds);

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void ExitState();

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void Restart();
};


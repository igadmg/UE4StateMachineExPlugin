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


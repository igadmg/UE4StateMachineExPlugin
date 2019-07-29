#pragma once

#include "StateMachine.generated.h"



UCLASS(blueprintable, BlueprintType)
class STATEMACHINEEX_API UStateMachine : public UObject
{
	GENERATED_BODY()



public:
	virtual class UWorld* GetWorld() const override;



public:
	UPROPERTY(Category = "State Machine", EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UState> ShutdownState;

	UPROPERTY(Category = "State Machine", EditAnywhere, BlueprintReadOnly)
	bool bImmediateStateChange = false;

	UPROPERTY(Category = "State Machine", VisibleInstanceOnly, BlueprintReadOnly)
	class UState *CurrentState;

	UPROPERTY(Category = "State Machine", VisibleInstanceOnly, BlueprintReadOnly)
	class UState *NextState;

	UPROPERTY(Category = "State Machine", VisibleInstanceOnly, BlueprintReadOnly)
	TArray<class UState*> StateStack;


public:
	UFUNCTION(Category = "State Machine", BlueprintCallable)
	bool IsActive() const;

	UFUNCTION(Category = "State Machine", BlueprintCallable)
	void Restart();

	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintNativeEvent)
	void Reset();

	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintNativeEvent)
	void Tick(float DeltaSeconds);

	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintNativeEvent)
	void Shutdown();



public:
	UStateMachine(const FObjectInitializer& ObjectInitializer);

	UState* SwitchState(TSubclassOf<class UState> NewStateClass);
	UState* SwitchState(class UState* NewState);
};

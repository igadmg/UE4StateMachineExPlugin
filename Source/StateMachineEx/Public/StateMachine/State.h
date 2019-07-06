#pragma once

#include "State.generated.h"



UCLASS(abstract, blueprintable, BlueprintType)
class STATEMACHINEEX_API UState : public UObject
{
	GENERATED_BODY()



public:
	virtual class UWorld* GetWorld() const override;



public:
	UPROPERTY(Category = "State Machine", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 StateId;

	UPROPERTY(Category = "State Machine", VisibleInstanceOnly, BlueprintReadOnly)
	class UStateMachine *ParentStateMachine;

	UPROPERTY(Category = "State Machine", VisibleInstanceOnly, BlueprintReadWrite)
	bool bPaused;



public:
	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintPure)
	virtual class APlayerController* GetOwningPlayer() const;

	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintPure)
	virtual class APawn* GetOwningPlayerPawn() const;



public:
	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void Enter();

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void Tick(float DeltaSeconds);

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void Exit();

	UFUNCTION(Category = "State Machine: State", BlueprintNativeEvent, BlueprintCallable)
	void Restart();



public:
	UState(const FObjectInitializer& ObjectInitializer);

	virtual void ConstructState(class UStateMachine *StateMachine)
	{
		ParentStateMachine = StateMachine;
	}
};

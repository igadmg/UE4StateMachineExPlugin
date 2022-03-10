#pragma once

#include "UObject/Object.h"

#include "StateMachine.generated.h"



UCLASS(blueprintable, BlueprintType)
class STATEMACHINEEX_API UStateMachine : public UObject
{
	GENERATED_BODY()



public:
	virtual class UWorld* GetWorld() const override;



public:
	UPROPERTY(Category = "State", EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UObject> ShutdownState;

	UPROPERTY(Category = "State", EditAnywhere, BlueprintReadOnly)
	bool bImmediateStateChange = false;

	UPROPERTY(Category = "State", VisibleInstanceOnly, BlueprintReadOnly)
	class UObject *CurrentState;

	UPROPERTY(Category = "State", VisibleInstanceOnly, BlueprintReadOnly)
	class UObject *NextState;

	UPROPERTY(Category = "State", VisibleInstanceOnly, BlueprintReadOnly)
	TArray<class UObject*> StateStack;


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

	UFUNCTION(Category = "State Machine", BlueprintCallable)
	UObject* SwitchState(class UObject* NewState);


public:
	UStateMachine(const FObjectInitializer& ObjectInitializer);

	UObject* PrepareState(TSubclassOf<class UObject> NewStateClass);
	UObject* SwitchStateByClass(TSubclassOf<class UObject> NewStateClass);
};

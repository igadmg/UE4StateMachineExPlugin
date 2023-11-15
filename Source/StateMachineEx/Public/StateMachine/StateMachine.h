#pragma once

#include "UObject/Object.h"
#include "Engine/EngineBaseTypes.h"

#include "StateMachine.generated.h"



USTRUCT()
struct FStateMachineTickFunction : public FTickFunction
{
	GENERATED_BODY()

	class UStateMachine* Target;

	/**
	* Abstract function to execute the tick.
	* @param DeltaTime - frame time to advance, in seconds.
	* @param TickType - kind of tick for this frame.
	* @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created.
	* @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completetion of this task until certain child tasks are complete.
	*/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph. */
	virtual FString DiagnosticMessage() override;
	/** Function used to describe this tick for active tick reporting. **/
	virtual FName DiagnosticContext(bool bDetailed) override;
};

template<>
struct TStructOpsTypeTraits<FStateMachineTickFunction> : public TStructOpsTypeTraitsBase2<FStateMachineTickFunction>
{
	enum
	{
		WithCopy = false
	};
};


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

	UPROPERTY(Category = "State", EditAnywhere)
	FStateMachineTickFunction AutoTickFunction;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	bool bIsAutoTickFunction = false;
#endif


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

	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintInternalUseOnly)
	UObject* K2Node_EnterState(class UObject* NewState);


public:
	UStateMachine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UObject* PrepareState(TSubclassOf<class UObject> NewStateClass);
	UObject* SwitchStateByClass(TSubclassOf<class UObject> NewStateClass);


protected:
	virtual void BeginDestroy() override;
};

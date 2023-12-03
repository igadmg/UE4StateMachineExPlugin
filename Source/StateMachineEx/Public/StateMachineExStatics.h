#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "StateMachineExStatics.generated.h"



UCLASS()
class STATEMACHINEEX_API UStateMachineExStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	UFUNCTION(Category = "StateMachineEx", BlueprintCallable)
	static class UStateMachine* SpawnStateMachine(UObject* Owner, TSubclassOf<class UStateMachine> StateMachineClass, bool bAutoTick = true);

	template <typename TStateMachine = class UStateMachine> static TStateMachine* SpawnStateMachine(UObject* Owner, bool bAutoTick = true)
	{
		return Cast<TStateMachine>(SpawnStateMachine(Owner, TStateMachine::StaticClass(), bAutoTick));
	}

	UFUNCTION(Category = "StateMachineEx", BlueprintPure, BlueprintInternalUseOnly, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static class UStateMachine* GuessStateMachineInternal(UObject* WorldContextObject);

	UFUNCTION(Category = "StateMachineEx", BlueprintPure, BlueprintInternalUseOnly, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static class UObject* GuessCurrentStateInternal(UObject* WorldContextObject);

	/** Used to get last state from state machine stack and switch to it. */
	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static void PushState(UObject* WorldContextObject);

	/** Used to put current state to state machine stack */
	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static void PopState(UObject* WorldContextObject);

	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, BlueprintInternalUseOnly, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static class UObject* CreateStateObject(UObject* WorldContextObject, UClass* StateClass);

	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (WorldContext = "WorldContextObject", DeterminesOutputType="StateClass"))
	static class UObject* EmbedStateObject(UObject* WorldContextObject, UClass* StateClass);
};

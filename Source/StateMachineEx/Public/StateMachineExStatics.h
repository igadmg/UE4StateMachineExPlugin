#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "StateMachineExStatics.generated.h"



UCLASS()
class STATEMACHINEEX_API UStateMachineExStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()



public:
	UFUNCTION(Category = "StateMachineEx", BlueprintPure, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static class UStateMachine* GuessStateMachine(UObject* WorldContextObject);

	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static void PushState(UObject* WorldContextObject);

	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static void PopState(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static class UObject* CreateStateObject(UObject* WorldContextObject, UClass* StateClass);
};

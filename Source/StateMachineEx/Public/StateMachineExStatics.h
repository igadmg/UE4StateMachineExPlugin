#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "StateMachineExStatics.generated.h"



UCLASS()
class STATEMACHINEEX_API UStateMachineExStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()



public:
	static class UStateMachine* GuessStateMachine(UObject* WorldContextObject);



public:
	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static void PushState(UObject* WorldContextObject);

	UFUNCTION(Category = "StateMachineEx", BlueprintCallable, meta = (HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static void PopState(UObject* WorldContextObject);
};

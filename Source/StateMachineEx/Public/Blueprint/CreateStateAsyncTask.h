#pragma once

#include "CreateStateAsyncTask.generated.h"



UCLASS()
class STATEMACHINEEX_API UCreateStateAsyncTask : public UObject
{
	GENERATED_BODY()



public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", HidePin = "WorldContextObject", WorldContext = "WorldContextObject"))
	static class UState* CreateStateObject(UObject* WorldContextObject, UClass* StateClass);



public:
	UCreateStateAsyncTask(const FObjectInitializer& ObjectInitializer);
};

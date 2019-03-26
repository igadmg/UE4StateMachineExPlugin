#pragma once

#include "ModuleManager.h"



class FStateMachineExModule : public IModuleInterface
{
public:
	static inline FStateMachineExModule& Get() { return FModuleManager::LoadModuleChecked<FStateMachineExModule>("StateMachineEx"); }
	static inline bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("StateMachineEx"); }

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};



struct FStateMachineEx
{
	// Declare module static functions here
};

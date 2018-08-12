#include "StateMachineDeveloperExPrivatePCH.h"
#include "Core.h"
#include "ModuleManager.h"



#define LOCTEXT_NAMESPACE "FStateMachineDeveloperExModule"



void FStateMachineDeveloperExModule::StartupModule()
{
}

void FStateMachineDeveloperExModule::ShutdownModule()
{
}



#undef LOCTEXT_NAMESPACE



IMPLEMENT_MODULE(FStateMachineDeveloperExModule, StateMachineDeveloperEx)

DEFINE_LOG_CATEGORY(LogStateMachineDeveloperEx);

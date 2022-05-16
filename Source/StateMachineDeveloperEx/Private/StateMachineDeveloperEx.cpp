#include "StateMachineDeveloperEx.h"

#include "StateMachine/State.h"
#include "ComponentAssetBroker.h"


#define LOCTEXT_NAMESPACE "FStateMachineDeveloperExModule"



class FStateComponentBroker : public IComponentAssetBroker
{
public:
	UClass* GetSupportedAssetClass() override
	{
		return UState::StaticClass();
	}

	virtual bool AssignAssetToComponent(UActorComponent* InComponent, UObject* InAsset) override
	{
#if false
		if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(InComponent))
		{
			UStaticMesh* StaticMesh = Cast<UStaticMesh>(InAsset);

			if ((StaticMesh != NULL) || (InAsset == NULL))
			{
				StaticMeshComp->SetStaticMesh(StaticMesh);
				return true;
			}
		}
#endif

		return false;
	}

	virtual UObject* GetAssetFromComponent(UActorComponent* InComponent) override
	{
#if false
		if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(InComponent))
		{
			return StaticMeshComp->GetStaticMesh();
		}
#endif

		return nullptr;
	}
};


void FStateMachineDeveloperExModule::StartupModule()
{
	//FComponentAssetBrokerage::RegisterBroker(MakeShareable(new FStaticMeshComponentBroker), UStaticMeshComponent::StaticClass(), true, true);
}

void FStateMachineDeveloperExModule::ShutdownModule()
{
}



#undef LOCTEXT_NAMESPACE



IMPLEMENT_MODULE(FStateMachineDeveloperExModule, StateMachineDeveloperEx)

DEFINE_LOG_CATEGORY(LogStateMachineDeveloperEx);

#pragma once

#include "StateMachine/State.h"
#include "PlayerStateState.generated.h"



UCLASS()
class STATEMACHINEEX_API UPlayerStateState : public UState
{
	GENERATED_BODY()


public:
	virtual class APlayerController* GetOwningPlayer() const override;
	virtual class APawn* GetOwningPlayerPawn() const override;

	UFUNCTION(Category = "State Machine", BlueprintCallable, BlueprintPure)
	virtual class APlayerState* GetPlayerState() const;



public:
	UPlayerStateState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

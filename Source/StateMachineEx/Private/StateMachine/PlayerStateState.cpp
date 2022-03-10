#include "StateMachine/PlayerStateState.h"
#include "StateMachineExPrivatePCH.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "StateMachine/StateMachine.h"


UPlayerStateState::UPlayerStateState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

APlayerController* UPlayerStateState::GetOwningPlayer() const
{
	const APawn* PlayerPawn = GetOwningPlayerPawn();

	return IsValid(PlayerPawn)
		? PlayerPawn->GetController<APlayerController>()
		: nullptr;
}

APawn* UPlayerStateState::GetOwningPlayerPawn() const
{
	const APlayerState* PlayerState = GetPlayerState();

	return IsValid(PlayerState)
		? PlayerState->GetPawn()
		: nullptr;
}

APlayerState* UPlayerStateState::GetPlayerState() const
{
	if (IsValid(ParentStateMachine))
	{
		return Cast<APlayerState>(ParentStateMachine->GetOuter());
	}

	return nullptr;
}
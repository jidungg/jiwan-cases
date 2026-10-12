#include "KnightAnimationControlComponent.h"
#include "Net/UnrealNetwork.h"
#include "PieceJumpMovementComponent.h"

void UKnightAnimationControlComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto* Movement = GetOwner()->FindComponentByClass<UPieceJumpMovementComponent>())
		Movement->BindOnJumpPhaseChanged(this, &UKnightAnimationControlComponent::OnJumpPhaseChanged);
}
void UKnightAnimationControlComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKnightAnimationControlComponent, CurrentJumpState);
}

void UKnightAnimationControlComponent::OnRep_KnightAnimState()
{
	ApplyKnightAnimState();
}

void UKnightAnimationControlComponent::OnJumpPhaseChanged(EJumpPhase NewPhase)
{
	CurrentJumpState = NewPhase;
	ApplyKnightAnimState();
}


void UKnightAnimationControlComponent::ApplyKnightAnimState()
{
}


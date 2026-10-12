#include "PieceAnimationControlComponent.h"
#include "SelectableComponent.h"
#include "../Pieces/ChessPieceBase.h"
#include "PieceMovementComponentBase.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UPieceAnimationControlComponent::UPieceAnimationControlComponent()
{
	OwnerPiece = nullptr;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UPieceAnimationControlComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPiece = Cast<AChessPieceBase>(GetOwner());
	if (nullptr == OwnerPiece)
	{
		return;
	}
	USkeletalMeshComponent* Skeletal = OwnerPiece->FindComponentByClass<USkeletalMeshComponent>();
	if (Skeletal)
		AnimInstance = Cast<UChessPieceAnimInstance>(Skeletal->GetAnimInstance());
	if (auto* Selectable = OwnerPiece->FindComponentByClass<USelectableComponent>())
		Selectable->BindOnSelectedChanged(this, &UPieceAnimationControlComponent::OnSelectedChanged);
	if (auto* Movement = OwnerPiece->FindComponentByClass<UPieceMovementComponentBase>())
		Movement->BindOnMovingChanged(this, &UPieceAnimationControlComponent::OnMovingChanged);
	OwnerPiece->BindOnMoveCommand(this, &UPieceAnimationControlComponent::OnMoveCommand);
}

void UPieceAnimationControlComponent::OnSelectedChanged(bool bNewSelected, APlayerController* Selector)
{
	if (!IsValid(AnimInstance) || nullptr == Selector)
	{
		return;
	}
	AnimInstance->bIsSelected = bNewSelected && OwnerPiece->IsMyPiece(Selector);
}

void UPieceAnimationControlComponent::OnMoveCommand(const FVector& Dest)
{
	SetAnimState(EPieceAnimState::Moving);
}

void UPieceAnimationControlComponent::OnRep_AnimState()
{
	ApplyAnimState();
}

void UPieceAnimationControlComponent::OnMovingChanged(bool bNewMoving)
{
	if (false == bNewMoving)
	{
		SetAnimState(EPieceAnimState::Idle);
	}
}

void UPieceAnimationControlComponent::SetAnimState(EPieceAnimState NewState)
{
	CurrentAnimState = NewState;
	ApplyAnimState();
}

void UPieceAnimationControlComponent::ApplyAnimState()
{
	if (!IsValid(AnimInstance))
		return;
	AnimInstance->CurrentAnimState = CurrentAnimState;
	OnAnimStateChanged.Broadcast(CurrentAnimState);
}

void UPieceAnimationControlComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPieceAnimationControlComponent, CurrentAnimState);
}


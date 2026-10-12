#include "PieceJumpMovementComponent.h"

void UPieceJumpMovementComponent::OnUnregister()
{
	OnJumpPhaseChanged.RemoveAll(this);
	Super::OnUnregister();
}

void UPieceJumpMovementComponent::MoveLogic(float DeltaTime)
{
	Super::MoveLogic(DeltaTime);
	if (EJumpPhase::Up == CurrentJumpPhase && Owner->GetActorLocation().Z >= JumpCeilingHeight)
	{
		FVector NewLocation = TargetLocation;
		NewLocation.Z = JumpCeilingHeight;
		Owner->SetActorLocation(NewLocation);
		SetJumpPhase(EJumpPhase::Down);
	}

}

FVector UPieceJumpMovementComponent::GetMoveDelta(const FVector& Current, const FVector& Target, float DeltaTime) const
{
	FVector Ret = {0.f, 0.f, 0.f};
	if (EJumpPhase::Up == CurrentJumpPhase)
		Ret.Z = 1.f;
	else if (EJumpPhase::Down == CurrentJumpPhase)
		Ret.Z = -1.f;
	Ret.Z *= DeltaTime * JumpPower;
	return Ret;
}

bool UPieceJumpMovementComponent::CheckArrival(const FVector& Current, const FVector& Target, const FVector& MoveDelta)
{
	double Step = FVector::Dist(FVector::ZeroVector, MoveDelta);
	float Remain = FVector::Dist(TargetLocation, Current);

	return Remain <= EpsilonDistance || Remain <= Step;
}
void UPieceJumpMovementComponent::StartMoveTo(const FVector& Dest)
{
	TargetLocation = Dest;
	JumpStartLocation = GetOwner()->GetActorLocation();
	SetJumpPhase(EJumpPhase::Ready);
	FTimerHandle ReadyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		ReadyTimerHandle, this,
		&UPieceJumpMovementComponent::OnJumpReadyEnded,
		JumpReadyTime, false
	);
	FRotator DestRot = (TargetLocation - JumpStartLocation).Rotation();
	DestRot.Pitch = 0.f;
	DestRot.Roll = 0.f;
	StartRotateTo(DestRot);
}


void UPieceJumpMovementComponent::OnArrival()
{
	StopMove(true);
	SetJumpPhase(EJumpPhase::Recover);
	FTimerHandle ReadyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		ReadyTimerHandle, this,
		&UPieceJumpMovementComponent::OnJumpRecoverEnded,
		JumpRecoverTime, false
	);
}

void UPieceJumpMovementComponent::OnJumpReadyEnded()
{
	Super::StartMoveTo(TargetLocation);
	SetJumpPhase(EJumpPhase::Up);
}

void UPieceJumpMovementComponent::OnJumpRecoverEnded()
{
	SetJumpPhase(EJumpPhase::None);
	if (bLookBaseAfterMove)
		StartRotateTo(BaseLookDirection.Rotation());
}

void UPieceJumpMovementComponent::SetJumpPhase(EJumpPhase JumpPhase)
{
	if (CurrentJumpPhase == JumpPhase)
		return;
	CurrentJumpPhase = JumpPhase;
	OnJumpPhaseChanged.Broadcast(CurrentJumpPhase);
}


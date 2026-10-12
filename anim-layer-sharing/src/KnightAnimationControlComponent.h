#pragma once

#include "CoreMinimal.h"
#include "PieceAnimationControlComponent.h"
#include "KnightAnimationControlComponent.generated.h"

UCLASS(ClassGroup = (Custom),meta = (BlueprintSpawnableComponent))
class RTCHESS_API UKnightAnimationControlComponent : public UPieceAnimationControlComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

	void OnJumpPhaseChanged(EJumpPhase JumpPhase);
	void ApplyKnightAnimState();
	UFUNCTION()
	void OnRep_KnightAnimState();
public:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_KnightAnimState, VisibleAnywhere, BlueprintReadWrite, Category = "AnimControl")
	EJumpPhase CurrentJumpState = EJumpPhase::None;
};

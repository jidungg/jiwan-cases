#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Animation/ChessPieceAnimInstance.h"
#include "PieceAnimationControlComponent.generated.h"

class AChessPieceBase;
DECLARE_MULTICAST_DELEGATE_OneParam(FAnimComponentEventDelegate, EPieceAnimState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RTCHESS_API UPieceAnimationControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPieceAnimationControlComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

public:
	virtual void OnSelectedChanged(bool bNewSelected, APlayerController* Selector);
	virtual void OnMovingChanged(bool bNewMoving);
	virtual void OnMoveCommand(const FVector& Dest);
	UFUNCTION()
	void OnRep_AnimState();

	template<typename UserClass>
	void BindOnAnimStateChanged(UserClass* Obj, void (UserClass::* InFunc)(EPieceAnimState));
protected:
	void SetAnimState(EPieceAnimState NewState);
	virtual void ApplyAnimState();
protected:
	AChessPieceBase* OwnerPiece;
	TObjectPtr<UChessPieceAnimInstance> AnimInstance;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_AnimState, VisibleAnywhere, BlueprintReadWrite, Category = "AnimControl")
	EPieceAnimState CurrentAnimState = EPieceAnimState::Idle;

	FAnimComponentEventDelegate OnAnimStateChanged;
};

template<typename UserClass>
inline void UPieceAnimationControlComponent::BindOnAnimStateChanged(UserClass* Obj, void(UserClass::* InFunc)(EPieceAnimState))
{
	OnAnimStateChanged.AddUObject(Obj, InFunc);
}

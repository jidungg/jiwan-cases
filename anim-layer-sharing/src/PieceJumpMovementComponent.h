#pragma once

#include "CoreMinimal.h"
#include "PieceMovementComponentBase.h"
#include "../Data/CommonDataHeader.h"
#include "PieceJumpMovementComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FJumpPhaseDelegate, EJumpPhase);

UCLASS(BlueprintType)
class RTCHESS_API UPieceJumpMovementComponent : public UPieceMovementComponentBase
{
	GENERATED_BODY()
public:
	virtual void OnUnregister() override;

	virtual void MoveLogic(float DeltaTime) override;
	virtual void StartMoveTo(const FVector& Dest);
	virtual FVector GetMoveDelta(const FVector& Current, const FVector& Target, float DeltaTime) const override;
	virtual bool CheckArrival(const FVector& Current, const FVector& Target, const FVector& MoveDelta) override;
	virtual void OnArrival() override;
	void OnJumpReadyEnded();
	void OnJumpRecoverEnded();
	template<typename UserClass>
	void BindOnJumpPhaseChanged(UserClass* Obj, void (UserClass::* InFunc)(EJumpPhase));

	void SetJumpPhase(EJumpPhase JumpPhase);
public:
	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpPower = 3000.f;
	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpReadyTime = 1.f;
	UPROPERTY(EditAnywhere, Category = "Jump")
	float JumpRecoverTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EJumpPhase CurrentJumpPhase;
private:
	FVector JumpStartLocation;
	FJumpPhaseDelegate OnJumpPhaseChanged;
	float JumpCeilingHeight = 2500.f;
};

template<typename UserClass>
inline void UPieceJumpMovementComponent::BindOnJumpPhaseChanged(UserClass* Obj, void(UserClass::* InFunc)(EJumpPhase))
{
	OnJumpPhaseChanged.AddUObject(Obj, InFunc);
}

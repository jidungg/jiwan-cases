#pragma once

#include "CoreMinimal.h"
#include "../Data/CommonDataHeader.h"
#include "Animation/AnimInstance.h"
#include "ChessPieceAnimInstance.generated.h"

UCLASS()
class RTCHESS_API UChessPieceAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimControl")
	bool bIsSelected = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimControl")
	EPieceAnimState CurrentAnimState = EPieceAnimState::Idle;
};

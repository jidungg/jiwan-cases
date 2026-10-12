#pragma once

#include "CoreMinimal.h"
#include "DogongGameModeBase.h"
#include "DogongRunGameMode.generated.h"

UCLASS()
class DOGONG_API ADogongRunGameMode : public ADogongGameModeBase
{
	GENERATED_BODY()

public:
	/** 접속한 플레이어에게 빈 기지 슬롯(0~3)을 배정하고 초기 지역을 그 기지로 설정. */
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};

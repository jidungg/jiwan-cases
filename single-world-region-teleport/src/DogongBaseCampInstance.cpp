#include "Systems/Region/DogongBaseCampInstance.h"
#include "GameState/DogongGameState.h"

void ADogongBaseCampInstance::BeginPlay()
{
	Super::BeginPlay();

	// 서버 권위: GameState 레지스트리에 자신을 등록해 슬롯 인덱스로 조회 가능하게 한다.
	if (HasAuthority())
	{
		if (ADogongGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADogongGameState>() : nullptr)
		{
			GS->RegisterBaseCampInstance(this);
		}
		SpawnPointLocal.SetLocation({100.f, 100.f, 100.f});
	}
}

#pragma once

#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "DogongBaseCampInstance.generated.h"

/**
 * 개인 기지 배치. BaseCamp.umap 원본 1개를 World.umap에 4개 배치해 쓰며,
 * 이 액터가 "몇 번 슬롯인지"와 "플레이어가 도착할 스폰 지점"을 든다.
 * 원본 애셋은 4개가 공유하므로 슬롯 구분/스폰은 배치(이 액터)에 스탬프된다.
 */
UCLASS()
class DOGONG_API ADogongBaseCampInstance : public ALevelInstance
{
	GENERATED_BODY()

public:
	/** 이 기지 배치의 슬롯 인덱스(0~3). RegionId(Region.Base.NN)의 근거. */
	int32 GetSlotIndex() const { return SlotIndex; }

	/** 플레이어가 이 기지로 텔레포트될 때 도착할 월드 트랜스폼. */
	FTransform GetSpawnTransform() const { return SpawnPointLocal * GetActorTransform(); }

protected:
	virtual void BeginPlay() override;

private:
	/** 슬롯 인덱스. 배치마다 0~3을 서로 다르게 지정한다. */
	UPROPERTY(EditInstanceOnly, Category = "BaseCamp", meta = (ClampMin = "0", ClampMax = "3"))
	int32 SlotIndex = INDEX_NONE;

	/** 스폰 지점(액터 로컬). 에디터 뷰포트 기즈모로 직접 조작 가능. */
	UPROPERTY(EditDefaultsOnly, Category = "BaseCamp", meta = (MakeEditWidget = true))
	FTransform SpawnPointLocal;
};

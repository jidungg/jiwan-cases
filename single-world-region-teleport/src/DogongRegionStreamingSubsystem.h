#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "DogongRegionStreamingSubsystem.generated.h"

class UDogongRegionTable;
class ULevelStreamingDynamic;

/** 지역 준비 완료 콜백. bSuccess=false면 로드 실패(진입 취소해야 함). */
DECLARE_DELEGATE_OneParam(FDogongOnRegionReady, bool /*bSuccess*/);

/** 지역 하나의 런타임 스트리밍 상태(서버 전용 부기). */
USTRUCT()
struct FDogongRegionRuntimeState
{
	GENERATED_BODY()

	/** 이 지역의 동적 스트리밍 인스턴스. */
	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> Instance = nullptr;

	/** 이 지역에 있는(진입한) 인원 수. 0이 되면 언로드. */
	int32 RefCount = 0;

	/** 레벨이 로드+가시화되어 진입 가능한 상태인지. */
	bool bReady = false;
};

/**
 * 약탈 지역을 온디맨드로 동적 스트리밍하는 서버 권위 서브시스템.
 * - 진입 시 RequestRegionReady로 로드 요청(refcount++), 준비되면 콜백.
 * - 이탈 시 ReleaseRegion으로 refcount--; 0이면 언로드(다음 진입 = 새 인스턴스 = 리셋).
 * 기지는 관장하지 않는다(ALevelInstance 상시 로드).
 */
UCLASS()
class DOGONG_API UDogongRegionStreamingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 지역 정의 테이블을 주입한다(서버 GameMode가 BeginPlay에서 호출). */
	void SetRegionTable(UDogongRegionTable* InTable) { RegionTable = InTable; }

	/**
	 * 지역을 준비(필요 시 동적 로드)하고 준비되면 OnReady(true) 콜백. refcount 증가.
	 * 로드 실패 시 OnReady(false). 이미 준비돼 있으면 즉시 콜백. 서버에서만 호출.
	 */
	void RequestRegionReady(const FGameplayTag& RegionId, FDogongOnRegionReady OnReady);

	/** 지역 사용 종료. refcount 감소, 0이면 언로드. */
	void ReleaseRegion(const FGameplayTag& RegionId);

	/** 지역 진입 스폰(월드 트랜스폼). 테이블에 없으면 false. */
	bool GetRegionEntryTransform(const FGameplayTag& RegionId, FTransform& OutTransform) const;

private:
	/** 인스턴스가 가시화(OnLevelShown)될 때 호출. 어느 지역인지는 인스턴스 가시성으로 판별. */
	UFUNCTION()
	void OnAnyRegionShown();

	/** RegionId의 대기 콜백들을 bSuccess로 발화하고, 실패면 상태를 정리한다. */
	void FireReady(const FGameplayTag& RegionId, bool bSuccess);

	/** RegionId에서 결정적 레벨 인스턴스 이름을 만든다(서버·클라 동일해야 복제됨). */
	static FString MakeInstanceName(const FGameplayTag& RegionId);

private:
	UPROPERTY()
	TObjectPtr<UDogongRegionTable> RegionTable = nullptr;

	UPROPERTY()
	TMap<FGameplayTag, FDogongRegionRuntimeState> Regions;

	/** 준비 대기 중인 콜백들(RegionId별). 델리게이트라 UPROPERTY 불가. */
	TMap<FGameplayTag, TArray<FDogongOnRegionReady>> PendingReady;
};

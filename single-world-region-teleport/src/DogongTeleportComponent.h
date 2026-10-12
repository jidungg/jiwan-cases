#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DogongTeleportComponent.generated.h"

class ADogongPlayerControllerBase;

/**
 * 삽입/탈출/지역이동 텔레포트 핸드셰이크를 오케스트레이션하는 플레이어별 컴포넌트.
 * ADogongPlayerControllerBase에 부착된다(서버 권위 상태머신).
 *  - 약탈 지역행: RegionStreamingSubsystem에 로드 요청 → 준비/실패/타임아웃 처리 → TeleportTo.
 *  - 기지행(탈출): 상시 로드된 기지 인스턴스로 대기 없이 이동.
 * 대기 중 소유 컨트롤러의 ClientStartLoading/ClientFinishLoading로 클라 로딩스크린을 제어한다.
 */
UCLASS(ClassGroup = (Dogong), meta = (BlueprintSpawnableComponent))
class DOGONG_API UDogongTeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * 서버 권위: 텔레포트 핸드셰이크를 시작한다.
	 * @param TargetRegionId   목적 약탈 지역(bToOwnBaseCamp=false일 때). 기지행이면 무시.
	 * @param bToOwnBaseCamp   true면 배정된 자기 기지로(탈출), false면 약탈 지역으로(삽입/이동).
	 */
	void BeginTeleport(const FGameplayTag& TargetRegionId, bool bToOwnBaseCamp);

private:
	/** 스트리밍 준비 완료/실패 콜백(서브시스템에서 호출). */
	void OnRegionReady(bool bSuccess);

	/** 스트리밍 대기 타임아웃. 재시도 소진 시 취소 후 제자리. */
	void OnTimeout();

	/** 실제 폰 이동 + 현재 지역 갱신 + 출발 지역 반납 + 로딩 종료. */
	void FinalizeTeleport(const FGameplayTag& TargetRegionId, const FTransform& Destination);

	/** 텔레포트 취소. 필요 시 요청했던 지역 refcount를 반납한다. */
	void AbortTeleport(bool bReleaseRequestedRegion);

	/** 진행 중 상태 초기화(공통). */
	void ResetState();

	ADogongPlayerControllerBase* GetOwnerController() const;

private:
	/** 진행 중 텔레포트가 있는지(서버 전용). */
	bool bTeleportInProgress = false;

	/** 스트리밍 요청한 약탈 지역(취소 시 반납 대상). */
	FGameplayTag PendingTargetRegion;

	/** 현재까지 타임아웃 재시도 횟수. */
	int32 TimeoutRetries = 0;

	FTimerHandle TimeoutTimer;

	/** 스트리밍 준비 대기 1회 타임아웃(초). */
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportTimeoutSeconds = 15.f;

	/** 타임아웃 시 대기를 연장할 최대 재시도 횟수. 소진하면 최종 취소. */
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	int32 MaxTimeoutRetries = 2;
};

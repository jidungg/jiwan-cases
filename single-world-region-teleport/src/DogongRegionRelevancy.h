#pragma once

#include "CoreMinimal.h"

class AActor;

/**
 * RegionId 기반 넷 렐러번시 격리 판정을 한 곳에 모은 헬퍼.
 * 격리대상 액터의 IsNetRelevantFor 오버라이드가 이 함수 하나만 호출하도록 해
 * 액터 타입마다 판정 로직이 흩어지지 않게 한다.
 */
namespace DogongRegion
{
	/**
	 * Self와 관찰자가 "서로 다른(둘 다 유효한)" 지역이면 true = 컬링 대상.
	 * 한쪽이라도 RegionId가 유효하지 않으면 false를 반환해 기본 거리 규칙에 맡긴다
	 * (배정 전 짧은 구간에서 과잉 컬링 방지).
	 * @param Self       렐러번시 판정 대상 액터(IDogongRegionProvider 구현).
	 * @param RealViewer 복제 수신 측(대개 PlayerController).
	 * @param ViewTarget 관찰자가 보는 액터(대개 조종 폰).
	 */
	DOGONG_API bool ShouldCullByRegion(const AActor* Self, const AActor* RealViewer, const AActor* ViewTarget);
}

#include "Systems/Region/DogongRegionRelevancy.h"
#include "Systems/Region/DogongRegionProvider.h"
#include "Player/DogongPlayerState.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

namespace DogongRegion
{
	static FGameplayTag GetActorRegion(const AActor* Actor)
	{
		if (const IDogongRegionProvider* Provider = Cast<IDogongRegionProvider>(Actor))
		{
			return Provider->GetRegionId();
		}
		return FGameplayTag();
	}

	static FGameplayTag GetViewerRegion(const AActor* RealViewer, const AActor* ViewTarget)
	{
		// 우선 ViewTarget(대개 조종 폰)이 지역 제공자면 그 값을 쓴다.
		if (const FGameplayTag TargetRegion = GetActorRegion(ViewTarget); TargetRegion.IsValid())
		{
			return TargetRegion;
		}
		// 폰이 없거나(사망 등) 무효면 RealViewer(컨트롤러)의 PlayerState.CurrentRegion으로 폴백.
		if (const APlayerController* PC = Cast<APlayerController>(RealViewer))
		{
			if (const ADogongPlayerState* PS = PC->GetPlayerState<ADogongPlayerState>())
			{
				return PS->GetCurrentRegion();
			}
		}
		return FGameplayTag();
	}

	bool ShouldCullByRegion(const AActor* Self, const AActor* RealViewer, const AActor* ViewTarget)
	{
		const FGameplayTag SelfRegion = GetActorRegion(Self);
		const FGameplayTag ViewerRegion = GetViewerRegion(RealViewer, ViewTarget);

		// 둘 다 유효하고 서로 다르면 컬링. 하나라도 무효면 기본 거리 규칙에 맡긴다.
		return SelfRegion.IsValid() && ViewerRegion.IsValid() && SelfRegion != ViewerRegion;
	}
}

#include "GameMode/DogongRunGameMode.h"
#include "GameState/DogongGameState.h"
#include "Player/DogongPlayerState.h"
#include "Systems/Region/DogongRegionStreamingSubsystem.h"
#include "DogongGameplayTags.h"

void ADogongRunGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	ADogongGameState* GS = GetGameState<ADogongGameState>();
	ADogongPlayerState* PS = NewPlayer->GetPlayerState<ADogongPlayerState>();
	if (!GS || !PS)
	{
		return;
	}

	const int32 Slot = GS->ClaimBaseCampSlot(PS);
	if (Slot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseCamp] 빈 기지 슬롯 없음 — 최대 %d명 초과 접속."),
			ADogongGameState::MaxBaseCampSlots);
		return;
	}

	// 배정 인덱스에서 RegionId 파생. 초기 지역 = 자기 기지.
	PS->SetAssignedBaseCampIndex(Slot);
	PS->SetCurrentRegion(ADogongGameState::BaseCampRegionTag(Slot));
}

void ADogongRunGameMode::Logout(AController* Exiting)
{
	// 슬롯 반납은 PS 포인터로 매칭한다. Super가 PlayerState를 정리하기 전에 캡처.
	ADogongPlayerState* PS = Exiting ? Cast<ADogongPlayerState>(Exiting->PlayerState) : nullptr;

	// Super::Logout이 PlayerState를 PlayerArray에서 제거한다. 총원 재계산이
	// 올바르려면 제거 후에 게이트에 통지해야 한다.
	Super::Logout(Exiting);

	if (PS)
	{
		if (ADogongGameState* GS = GetGameState<ADogongGameState>())
		{
			GS->ReleaseBaseCampSlot(PS);
		}

		// 약탈 지역에 있던 채로 나갔다면 그 지역 refcount를 반납해야 언로드가 성립한다(누수 방지).
		const FGameplayTag Region = PS->GetCurrentRegion();
		if (Region.IsValid() && Region.MatchesTag(DogongTags::Region_Raid))
		{
			if (UWorld* World = GetWorld())
			{
				if (UDogongRegionStreamingSubsystem* Sub = World->GetSubsystem<UDogongRegionStreamingSubsystem>())
				{
					Sub->ReleaseRegion(Region);
				}
			}
		}
	}
}


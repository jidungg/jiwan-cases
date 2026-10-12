#include "Systems/Region/DogongTeleportComponent.h"
#include "Systems/Region/DogongRegionStreamingSubsystem.h"
#include "Systems/Region/DogongBaseCampInstance.h"
#include "Player/DogongPlayerControllerBase.h"
#include "Player/DogongPlayerState.h"
#include "GameState/DogongGameState.h"
#include "DogongGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADogongPlayerControllerBase* UDogongTeleportComponent::GetOwnerController() const
{
	return Cast<ADogongPlayerControllerBase>(GetOwner());
}

void UDogongTeleportComponent::BeginTeleport(const FGameplayTag& TargetRegionId, bool bToOwnBaseCamp)
{
	ADogongPlayerControllerBase* PC = GetOwnerController();
	if (!PC || !PC->HasAuthority())
	{
		return;
	}
	if (bTeleportInProgress)
	{
		return; // 이미 진행 중 — 중복 상호작용 무시
	}

	APawn* Pawn = PC->GetPawn();
	ADogongPlayerState* PS = PC->GetPlayerState<ADogongPlayerState>();
	UWorld* World = PC->GetWorld();
	if (!Pawn || !PS || !World)
	{
		return;
	}

	if (bToOwnBaseCamp)
	{
		ADogongGameState* GS = World->GetGameState<ADogongGameState>();
		const int32 Idx = PS->GetAssignedBaseCampIndex();
		ADogongBaseCampInstance* Inst = GS ? GS->GetBaseCampInstance(Idx) : nullptr;
		if (!Inst)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Teleport] 기지 인스턴스 없음(slot=%d) — 탈출 불가."), Idx);
			return;
		}

		// 기지는 상시 로드 → 스트리밍 대기 없이 즉시 이동. 로딩스크린은 팝 은폐용.
		bTeleportInProgress = true;
		PC->ClientStartLoading();
		FinalizeTeleport(ADogongGameState::BaseCampRegionTag(Idx), Inst->GetSpawnTransform());
		return;
	}

	// 약탈 지역: 온디맨드 스트리밍 대기.
	UDogongRegionStreamingSubsystem* Sub = World->GetSubsystem<UDogongRegionStreamingSubsystem>();
	if (!Sub || !TargetRegionId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Teleport] 스트리밍 서브시스템/RegionId 없음 — 삽입 불가."));
		return;
	}

	bTeleportInProgress = true;
	PendingTargetRegion = TargetRegionId;
	TimeoutRetries = 0;

	PC->ClientStartLoading();
	World->GetTimerManager().SetTimer(TimeoutTimer, this, &UDogongTeleportComponent::OnTimeout, TeleportTimeoutSeconds, false);

	Sub->RequestRegionReady(TargetRegionId,
		FDogongOnRegionReady::CreateUObject(this, &UDogongTeleportComponent::OnRegionReady));
}

void UDogongTeleportComponent::OnRegionReady(bool bSuccess)
{
	if (!bTeleportInProgress)
	{
		return; // 이미 타임아웃/취소로 종료됨
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeoutTimer);
	}

	if (!bSuccess)
	{
		// 서브시스템이 실패 시 상태(refcount 포함)를 이미 정리 → 추가 release 불필요.
		AbortTeleport(false);
		return;
	}

	UWorld* World = GetWorld();
	UDogongRegionStreamingSubsystem* Sub = World ? World->GetSubsystem<UDogongRegionStreamingSubsystem>() : nullptr;
	FTransform Dest;
	if (!Sub || !Sub->GetRegionEntryTransform(PendingTargetRegion, Dest))
	{
		AbortTeleport(true);
		return;
	}

	FinalizeTeleport(PendingTargetRegion, Dest);
}

void UDogongTeleportComponent::OnTimeout()
{
	if (!bTeleportInProgress)
	{
		return;
	}

	// 아직 로딩 중일 수 있으니, 재시도 여유가 있으면 대기를 연장한다.
	if (TimeoutRetries < MaxTimeoutRetries)
	{
		++TimeoutRetries;
		UE_LOG(LogTemp, Warning, TEXT("[Teleport] 지역 %s 스트리밍 지연 — 대기 연장 %d/%d."),
			*PendingTargetRegion.ToString(), TimeoutRetries, MaxTimeoutRetries);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(TimeoutTimer, this, &UDogongTeleportComponent::OnTimeout, TeleportTimeoutSeconds, false);
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Teleport] 지역 %s 스트리밍 타임아웃 — 취소 후 제자리."),
		*PendingTargetRegion.ToString());
	AbortTeleport(true);
}

void UDogongTeleportComponent::FinalizeTeleport(const FGameplayTag& TargetRegionId, const FTransform& Destination)
{
	ADogongPlayerControllerBase* PC = GetOwnerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	ADogongPlayerState* PS = PC ? PC->GetPlayerState<ADogongPlayerState>() : nullptr;
	UWorld* World = PC ? PC->GetWorld() : nullptr;

	if (Pawn && PS)
	{
		const FGameplayTag OldRegion = PS->GetCurrentRegion();

		Pawn->TeleportTo(Destination.GetLocation(), Destination.Rotator(), false, true);
		PS->SetCurrentRegion(TargetRegionId);

		// 출발 지역이 약탈 지역이었다면 refcount 반납(마지막 이탈이면 언로드 = 리셋).
		if (World && OldRegion.IsValid() && OldRegion != TargetRegionId
			&& OldRegion.MatchesTag(DogongTags::Region_Raid))
		{
			if (UDogongRegionStreamingSubsystem* Sub = World->GetSubsystem<UDogongRegionStreamingSubsystem>())
			{
				Sub->ReleaseRegion(OldRegion);
			}
		}
	}

	if (PC)
	{
		PC->ClientFinishLoading();
	}
	ResetState();
}

void UDogongTeleportComponent::AbortTeleport(bool bReleaseRequestedRegion)
{
	if (bReleaseRequestedRegion && PendingTargetRegion.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			if (UDogongRegionStreamingSubsystem* Sub = World->GetSubsystem<UDogongRegionStreamingSubsystem>())
			{
				Sub->ReleaseRegion(PendingTargetRegion);
			}
		}
	}

	if (ADogongPlayerControllerBase* PC = GetOwnerController())
	{
		PC->ClientFinishLoading();
		// TODO: 클라 에러 UI(별도 RPC). 현재는 로딩스크린만 내린다.
	}
	ResetState();
}

void UDogongTeleportComponent::ResetState()
{
	bTeleportInProgress = false;
	PendingTargetRegion = FGameplayTag();
	TimeoutRetries = 0;
}

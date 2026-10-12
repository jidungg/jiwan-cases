#include "Systems/Region/DogongRegionStreamingSubsystem.h"
#include "Systems/Region/DogongRegionTable.h"
#include "Engine/LevelStreamingDynamic.h"

FString UDogongRegionStreamingSubsystem::MakeInstanceName(const FGameplayTag& RegionId)
{
	// "Region.Raid.01" → "Region_Raid_01". 서버·클라가 같은 이름을 써야 복제된다.
	FString Name = RegionId.ToString();
	Name.ReplaceInline(TEXT("."), TEXT("_"));
	return Name;
}

void UDogongRegionStreamingSubsystem::RequestRegionReady(const FGameplayTag& RegionId, FDogongOnRegionReady OnReady)
{
	FDogongRegionRuntimeState& State = Regions.FindOrAdd(RegionId);
	State.RefCount++;

	// 이미 준비됨 → 즉시 콜백.
	if (State.bReady)
	{
		OnReady.ExecuteIfBound(true);
		return;
	}

	// 준비 대기열에 등록.
	PendingReady.FindOrAdd(RegionId).Add(OnReady);

	// 이미 로딩 중이면 대기.
	if (State.Instance)
	{
		return;
	}

	// 신규 로드 시작.
	const FDogongRegionDef* Def = RegionTable ? RegionTable->FindByRegionId(RegionId) : nullptr;
	if (!Def || Def->Level.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Region] RegionId %s 정의를 테이블에서 못 찾음 — 로드 실패."),
			*RegionId.ToString());
		FireReady(RegionId, false);
		return;
	}

	bool bSuccess = false;
	ULevelStreamingDynamic* Inst = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this, Def->Level, Def->PlacementTransform, bSuccess, MakeInstanceName(RegionId));

	if (!bSuccess || !Inst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Region] RegionId %s 스트리밍 인스턴스 생성 실패."),
			*RegionId.ToString());
		FireReady(RegionId, false);
		return;
	}

	State.Instance = Inst;
	Inst->OnLevelShown.AddDynamic(this, &UDogongRegionStreamingSubsystem::OnAnyRegionShown);
}

void UDogongRegionStreamingSubsystem::ReleaseRegion(const FGameplayTag& RegionId)
{
	FDogongRegionRuntimeState* State = Regions.Find(RegionId);
	if (!State)
	{
		return;
	}

	State->RefCount = FMath::Max(0, State->RefCount - 1);
	if (State->RefCount > 0)
	{
		return;
	}

	// 마지막 이탈 → 언로드(다음 진입 시 새 인스턴스 = 리셋).
	if (State->Instance)
	{
		State->Instance->OnLevelShown.RemoveDynamic(this, &UDogongRegionStreamingSubsystem::OnAnyRegionShown);
		State->Instance->SetShouldBeVisible(false);
		State->Instance->SetShouldBeLoaded(false);
	}
	Regions.Remove(RegionId);
	PendingReady.Remove(RegionId);
}

bool UDogongRegionStreamingSubsystem::GetRegionEntryTransform(const FGameplayTag& RegionId, FTransform& OutTransform) const
{
	const FDogongRegionDef* Def = RegionTable ? RegionTable->FindByRegionId(RegionId) : nullptr;
	if (!Def)
	{
		OutTransform = FTransform::Identity;
		return false;
	}
	OutTransform = Def->GetWorldEntryTransform();
	return true;
}

void UDogongRegionStreamingSubsystem::OnAnyRegionShown()
{
	// no-param 콜백이라 어느 지역이 가시화됐는지는 인스턴스 가시성으로 판별한다.
	// 델리게이트 발화가 Regions 맵을 변형할 수 있으므로 키를 먼저 수집한 뒤 발화.
	TArray<FGameplayTag> NowReady;
	for (const TPair<FGameplayTag, FDogongRegionRuntimeState>& Pair : Regions)
	{
		const FDogongRegionRuntimeState& State = Pair.Value;
		if (!State.bReady && State.Instance && State.Instance->IsLevelVisible())
		{
			NowReady.Add(Pair.Key);
		}
	}

	for (const FGameplayTag& RegionId : NowReady)
	{
		FireReady(RegionId, true);
	}
}

void UDogongRegionStreamingSubsystem::FireReady(const FGameplayTag& RegionId, bool bSuccess)
{
	if (bSuccess)
	{
		if (FDogongRegionRuntimeState* State = Regions.Find(RegionId))
		{
			State->bReady = true;
		}
	}

	// 재진입 방지를 위해 대기열을 지역 변수로 옮긴 뒤 발화.
	TArray<FDogongOnRegionReady> Local;
	if (TArray<FDogongOnRegionReady>* List = PendingReady.Find(RegionId))
	{
		Local = MoveTemp(*List);
		PendingReady.Remove(RegionId);
	}
	for (FDogongOnRegionReady& Delegate : Local)
	{
		Delegate.ExecuteIfBound(bSuccess);
	}

	// 로드 실패 → 아무것도 로드되지 않았으므로 상태 제거.
	if (!bSuccess)
	{
		Regions.Remove(RegionId);
	}
}

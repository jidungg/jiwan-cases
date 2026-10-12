#include "RTChessPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h" // 추가

ARTChessPlayerState::ARTChessPlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
	SetNetUpdateFrequency(1.f);
	SetMinNetUpdateFrequency(1.f);
}
void ARTChessPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARTChessPlayerState, EnergySync);
}
void ARTChessPlayerState::BeginPlay()
{
	Super::BeginPlay();
	CachedGS = GetWorld()->GetGameState();
	if (!CachedGS)
		return;
	if (HasAuthority())
	{
		EnergySync.Energy = MaxEnergy;
		EnergySync.ServerTime = CachedGS->GetServerWorldTimeSeconds();
		ForceNetUpdate();
	}
}

void ARTChessPlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		if (EnergySync.Energy >= MaxEnergy)
			return;

		EnergySync.Energy = FMath::Clamp(EnergySync.Energy + EnergyRegenSpeed * DeltaSeconds, 0.f, MaxEnergy);
		EnergySync.ServerTime = CachedGS->GetServerWorldTimeSeconds();
	}
	else
	{
		OnEnergyRateChanged.Broadcast(GetEnergyDisplayed() / MaxEnergy);
	}

}
//클라이언트에게 보여지는 기력값 계산 함수
float ARTChessPlayerState::GetEnergyDisplayed() const
{
	return FMath::Clamp(GetEnergyPredicted() - PendingLocalConsume, 0.f, MaxEnergy);
}
//클라이언트 기력 예측 함수
float ARTChessPlayerState::GetEnergyPredicted() const
{
	if (!CachedGS) return EnergySync.Energy;

	const float NowServer = CachedGS->GetServerWorldTimeSeconds();
	const float dt = FMath::Max(0.f, NowServer - EnergySync.ServerTime);

	const float Pred = EnergySync.Energy + EnergyRegenSpeed * dt;
	return FMath::Clamp(Pred, 0.f, MaxEnergy);
}

//기력 소비 Server RPC
void ARTChessPlayerState::ConsumeEnergy_Implementation()
{
	if (false == HasAuthority())
		return;
	if (EnergyConsumption > EnergySync.Energy)
		return;
	EnergySync.Energy = FMath::Clamp(EnergySync.Energy - EnergyConsumption, 0.f, MaxEnergy);
	EnergySync.ServerTime = CachedGS->GetServerWorldTimeSeconds();
	EnergySync.ConfirmedConsumeSeq++;
	ForceNetUpdate();
}
bool ARTChessPlayerState::HasEnoughEnergy() const
{
	if (HasAuthority())
		return EnergySync.Energy >= EnergyConsumption;

	return GetEnergyDisplayed() >= EnergyConsumption;
}
void ARTChessPlayerState::OnRep_Energy()
{
	uint32 Acked = EnergySync.ConfirmedConsumeSeq - LastConfirmedConsumeSeq;
	LastConfirmedConsumeSeq = EnergySync.ConfirmedConsumeSeq;
	PendingLocalConsume = FMath::Max(0.f, PendingLocalConsume - Acked * EnergyConsumption);
	OnEnergyRateChanged.Broadcast(GetEnergyDisplayed() / MaxEnergy);
}

void ARTChessPlayerState::ApplyLocalPredictedConsumeEnergy()
{
	if (false == HasEnoughEnergy())
		return;
	PendingLocalConsume = FMath::Clamp(PendingLocalConsume + EnergyConsumption, 0.f, MaxEnergy);
	OnEnergyRateChanged.Broadcast(GetEnergyDisplayed() / MaxEnergy);
}
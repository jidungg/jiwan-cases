#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RTChessPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnergyChanged, float);

USTRUCT()
struct FEnergySync
{
	GENERATED_BODY()
public:
	UPROPERTY()
	float Energy = 0.f;
	UPROPERTY()
	float ServerTime = 0.f;
	UPROPERTY()
	uint32 ConfirmedConsumeSeq = 0;
};


UCLASS()
class RTCHESS_API ARTChessPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ARTChessPlayerState();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	UFUNCTION()
	void OnRep_Energy();

	bool HasEnoughEnergy() const;
	float GetEnergyDisplayed() const;
	float GetEnergyPredicted() const;
	void ApplyLocalPredictedConsumeEnergy();
	void ConsumeEnergy_Implementation();
	UFUNCTION(Server, Reliable)
	void ConsumeEnergy();
	template<typename UserClass>
	void BindOnEnergyChanged(UserClass* Obj, void (UserClass::*InFunc)(float))
	{
		OnEnergyRateChanged.AddUObject(Obj, InFunc);
	}
private:
	UPROPERTY(EditDefaultsOnly, Category = "Energy")
	float MaxEnergy = 4.f;
	UPROPERTY(EditDefaultsOnly, Category = "Energy")
	float EnergyRegenSpeed = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "Energy")
	float EnergyConsumption = 1.f;
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Energy)
	FEnergySync EnergySync;
	FOnEnergyChanged OnEnergyRateChanged;

	const AGameStateBase* CachedGS;

	uint32 LastConfirmedConsumeSeq = 0;
	float PendingLocalConsume = 0.f;
};


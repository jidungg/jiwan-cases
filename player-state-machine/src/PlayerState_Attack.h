#pragma once
#include "PlayerState.h"

BEGIN(Client)
class CPlayerState_Attack :
    public CPlayerState
{
public:
    // CPlayerState을(를) 통해 상속됨
    void Update(_float _fTimeDelta) override;
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void On_AnimEnd(COORDINATE _eCoord, _uint _iAnimIdx) override;
private:
    void Switch_To_AttackAnimation(_uint _iComboCount);
private:
    static constexpr _uint iMaxComboCount = 3;

    _uint m_iComboCount = 0;
    _bool m_bCombo = false;
    _float m_f2DForwardSpeed = 0.f;
    _float m_f2DForwardStartProgress = 0.1f;
    _float m_f2DForwardEndProgress = 0.25f;

    _float m_f3DMotionCancelProgress[iMaxComboCount] = { 0.35f,0.35f,0.5f };
    _float m_f2DMotionCancelProgress[iMaxComboCount] = { 0.5f,0.5f,0.5f };

    //PlatformerMode 에서 사용
    _float m_f2DRisingForce = 200.f;
    _bool m_bRised = false;

    _float m_fAttackSfxDelay = 0.15f;
    _float m_fVocalSfxVolume = 25.f;
    _float m_fSwordSfxVolume = 40.f;
    _int m_iSmallEffortsSfxCount = 5;
    _int m_iSwordSwingSfxCount = 5;
    _int m_iSwordStrike2SfxCount = 6;
    _int m_iSwordStrike3SfxCount = 6;
    _int m_iFinalBlowSfxCount = 16;
};

END
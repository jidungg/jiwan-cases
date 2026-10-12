#pragma once
#include "PlayerState.h"

BEGIN(Client)
class CPlayerState_Idle :
    public CPlayerState
{
public:
    // CPlayerState을(를) 통해 상속됨
    void Update(_float _fTimeDelta) override;
    virtual void Enter() override;
    virtual void Exit() override;
    void Switch_IdleAnimation2D(F_DIRECTION _eFDir);
private:
    void Switch_IdleAnimation3D(_bool _bStealth);
private:
    _bool m_bSneakBefore = false;
    _bool m_bPlatformerMode = false;
};

END
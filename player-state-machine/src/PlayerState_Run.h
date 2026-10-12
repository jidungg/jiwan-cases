#pragma once
#include "PlayerState.h"

BEGIN(Client)
class CPlayerState_Run :
    public CPlayerState
{
public:
    // CPlayerState을(를) 통해 상속됨
    void Update(_float _fTimeDelta) override;
    virtual void Enter() override;
    virtual void Exit() override;

    void Switch_RunAnimation2D(F_DIRECTION _eFDir);
    void Switch_RunAnimation3D(_bool _bStealth);

private:
    _float m_fRotateSpeed = 1080.f;
    _bool m_bSneakBefore = false;
    _bool m_bPlatformerMode = false;
    F_DIRECTION m_eOldFDir = F_DIRECTION::F_DIR_LAST;
    _float m_fAccSoundDelay = 0.f;
};

END
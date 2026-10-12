#pragma once
#include "Base.h"
#include "AnimEventGenerator.h"


BEGIN(Engine)
class ENGINE_DLL CAnimation abstract:
    public CBase
{
public:
    virtual _float Get_Progress() abstract;

    virtual void Set_Progress(_float _fProgress, _bool _bReverse) abstract;
    void Set_SpeedMagnifier(_float _fMag) { m_fSpeedMagnifier = _fMag; }
    void Set_Loop(_bool _bIsLoop) { m_bLoop = _bIsLoop; }
    virtual void Reset(_bool _bReverse = false);
    virtual _float Get_AnimationTime() abstract;

    virtual void RegisterAnimEvent(ANIM_EVENT* _pAnimEvent) { m_AnimEvents.push_back(_pAnimEvent); }
protected:
    _float m_fSpeedMagnifier = 1.f;
    _bool m_bLoop = false;
protected:
    vector<ANIM_EVENT*> m_AnimEvents;
};

END
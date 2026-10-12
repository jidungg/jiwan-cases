#pragma once
#include "Model.h"
#include "Animation2D.h"


BEGIN(Engine)
class CVIBuffer_Rect;
class ENGINE_DLL C2DModel :
    public CModel
{
public:
    virtual HRESULT			Render(CShader* _pShader, _uint _iShaderPass)override;
    virtual _bool			Play_Animation(_float _fTimeDelta, _bool _bReverse)override;
    //Get
    virtual _uint Get_CurrentAnimIndex() override { return m_iCurAnimIdx; }
    virtual _float Get_CurrentAnimProgeress() override;

    //Set
    virtual void Set_AnimationLoop(_uint _iIdx, _bool _bIsLoop)override;
    virtual void Set_Progress(_uint _iIdx, _float _fProgress, _bool _bReverse) override;
    virtual void Switch_Animation(_uint _iIdx, _bool _bReverse = false)override;
    virtual void Set_AnimSpeedMagnifier(_uint _iAnimIndex, _float _fMag) override;

protected:
    CVIBuffer_Rect* m_pVIBufferCom = nullptr;

    vector<CAnimation2D*> m_Animation2Ds;

    CSpriteFrame* m_pNonAnimSprite = nullptr;

    _uint m_iCurAnimIdx = 0;
};

END
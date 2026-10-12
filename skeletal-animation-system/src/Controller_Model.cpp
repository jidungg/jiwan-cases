#include "Controller_Model.h"
#include "GameInstance.h"

HRESULT CController_Model::Change_Coordinate(COORDINATE _eCoordinate)
{
    m_eCurCoord = _eCoordinate;

    return S_OK;
}

void CController_Model::Play_Animation(_float _fTimeDelta, _bool _bReverse)
{
    if (m_ModelComs[m_eCurCoord])
    {
        if (m_ModelComs[m_eCurCoord]->Is_AnimModel())
        {
            if (m_ModelComs[m_eCurCoord]->Play_Animation(_fTimeDelta, _bReverse))
            {
                Invoke_OnAnimEndCallBack(m_eCurCoord, m_ModelComs[m_eCurCoord]->Get_CurrentAnimIndex());
            }
        }
        else
            m_ModelComs[m_eCurCoord]->Set_DuringAnimation(false);

    }
}


void CController_Model::Invoke_OnAnimEndCallBack(COORDINATE _eCoord, _uint _iAnimIdx)
{
    for (auto& callback : m_listAnimEndCallBack)
        callback(_eCoord, _iAnimIdx);
}

void CController_Model::Set_AnimationLoop(COORDINATE _eCoord, _uint _iIdx, _bool _bIsLoop)
{
    m_ModelComs[_eCoord]->Set_AnimationLoop(_iIdx, _bIsLoop);
}

void CController_Model::Switch_Animation(_uint _iIdx, _bool _bReverse)
{
    m_ModelComs[m_eCurCoord]->Switch_Animation(_iIdx, _bReverse);
}

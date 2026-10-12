#include "2DModel.h"
#include "GameInstance.h"
#include "Engine_Defines.h"

void C2DModel::Set_AnimationLoop(_uint _iIdx, _bool _bIsLoop)
{
	_int iTemp = (_int)m_Animation2Ds.size() - 1;
	if (iTemp < (_int)_iIdx)
	{
		return;
	}
	m_Animation2Ds[_iIdx]->Set_Loop(_bIsLoop);
}

void C2DModel::Set_Progress(_uint _iIdx, _float _fProgress, _bool _bReverse)
{
	_int iSize = m_Animation2Ds.size() - 1;
	if (iSize < (_int)_iIdx)
	{
		return;
	}
	m_Animation2Ds[_iIdx]->Set_Progress(_fProgress, _bReverse);
}

void C2DModel::Switch_Animation(_uint _iIdx, _bool _bReverse)
{
	_int iTemp = (_int)m_Animation2Ds.size() - 1;
	if (iTemp < (_int)_iIdx)
	{
		return;
	}
	m_iCurAnimIdx = _iIdx;
	m_Animation2Ds[m_iCurAnimIdx]->Reset(_bReverse);
}

void C2DModel::Set_AnimSpeedMagnifier(_uint _iAnimIndex, _float _fMag)
{
	if (_iAnimIndex >= m_Animation2Ds.size())
	{
		return;
	}
	m_Animation2Ds[_iAnimIndex]->Set_SpeedMagnifier(_fMag);
}

HRESULT C2DModel::Render(CShader* _pShader, _uint _iShaderPass)
{
	if (Is_AnimModel())
	{
		if (m_Animation2Ds[m_iCurAnimIdx])
			if (FAILED(m_Animation2Ds[m_iCurAnimIdx]->Bind_ShaderResource(_pShader)))
				return E_FAIL;
	}
	else
	{
		assert(m_pNonAnimSprite);
		if (FAILED(m_pNonAnimSprite->Bind_ShaderResource(_pShader)))
			return E_FAIL;
	}


	if (_pShader)
		if (FAILED(_pShader->Begin(_iShaderPass)))
			return E_FAIL;

	if (m_pVIBufferCom)
	{
		m_pVIBufferCom->Bind_BufferDesc();
		m_pVIBufferCom->Render();
	}
	else
		return E_FAIL;

	return S_OK;
}

_bool C2DModel::Play_Animation(_float _fTimeDelta, _bool _bReverse)
{
	m_bReverseAnimation = _bReverse;
	if (Is_AnimModel())
	{
		_bool bAnimEnd = m_Animation2Ds[m_iCurAnimIdx]->Play_Animation(_fTimeDelta, _bReverse);
		m_bDuringAnimation = (false == bAnimEnd);
		return bAnimEnd;
	}
	else
		m_bDuringAnimation = false;
	return false;
}

_float C2DModel::Get_CurrentAnimProgeress()
{
	return m_Animation2Ds[m_iCurAnimIdx]->Get_Progress();
}

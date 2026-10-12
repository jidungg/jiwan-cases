#include "stdafx.h"
#include "EffFlipController.h"
#include "EffTexturingProperty.h"

HRESULT CEffFlipController::Initialize_Prototype(ifstream& inFile, const CEffModel* pModel)
{
	m_eCtrlType = EFF_CTRL_TYPE::CT_TEXTURE_FLIP;
	if (FAILED(__super::Initialize_Prototype(inFile, pModel)))
		return E_FAIL;
	inFile.read(reinterpret_cast<char*>(&m_eTexSlot), sizeof(_uint));
	_uint iCount = 0;
	inFile.read(reinterpret_cast<char*>(&iCount), sizeof(_uint));
	for (_uint i = 0; i < iCount; i++)
	{
		FLOAT_KEYFRAME tNewFrame;
		inFile.read(reinterpret_cast<char*>(&tNewFrame.fValue), sizeof(_float));
		inFile.read(reinterpret_cast<char*>(&tNewFrame.fTrackPosition), sizeof(_float));
		m_vecKeyFrame.push_back(tNewFrame);
	}

	inFile.read(reinterpret_cast<char*>(&iCount), sizeof(_uint));
	vecSourceTextureIndex.resize(iCount);
	for (_uint i = 0; i < iCount; i++)
	{
		inFile.read(reinterpret_cast<char*>(&vecSourceTextureIndex[i]), sizeof(_uint));
	}

	return S_OK;
}

_bool CEffFlipController::Update_InTime(_float fTrackPos)
{
	assert(m_pTarget);
	CEffTexturingProperty* pTarget = static_cast<CEffTexturingProperty*>(m_pTarget);
	//GetCurrentKeyFrame : fTrackPos를 넘지 않는 가장 가까운 KeyFrame
	_uint iCurrentKeyFrameIndex = 0;
	_uint iNumFrame = m_vecKeyFrame.size();
	if (iNumFrame == 1)
	{
		pTarget->Set_TextureIndex(m_eTexSlot, vecSourceTextureIndex[(_uint)m_vecKeyFrame[0].fValue]);
		return true;
	}
	if (m_vecKeyFrame[iNumFrame - 1].fTrackPosition < fTrackPos)
	{
		pTarget->Set_TextureIndex(m_eTexSlot, vecSourceTextureIndex[(_int)m_vecKeyFrame[iNumFrame - 1].fValue]);
		return true;
	}
	while (m_vecKeyFrame[iCurrentKeyFrameIndex + 1].fTrackPosition < fTrackPos)
	{
		iCurrentKeyFrameIndex++;
		//마지막 프레임보다 많이 갔으면? 뭔가 이상함

	}

	FLOAT_KEYFRAME tLeftKeyFrame = m_vecKeyFrame[iCurrentKeyFrameIndex];
	FLOAT_KEYFRAME tRightKeyFrame = m_vecKeyFrame[iCurrentKeyFrameIndex + 1];


	_float fRatio = (fTrackPos - tLeftKeyFrame.fTrackPosition)
		/ (tRightKeyFrame.fTrackPosition - tLeftKeyFrame.fTrackPosition);

	_float fValue = (tRightKeyFrame.fValue - tLeftKeyFrame.fValue) * fRatio + tLeftKeyFrame.fValue;
	pTarget->Set_TextureIndex(m_eTexSlot, vecSourceTextureIndex[fValue]);
	return false;
}

void CEffFlipController::Set_Target(vector<CEffTexturingProperty*>& vecTexturing)
{
	m_pTarget = vecTexturing[m_iTargetIndex];
}

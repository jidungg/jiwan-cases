#include "..\Public\Channel.h"
#include "3DModel.h"
#include "Bone.h"
#include "Animation3D.h"

HRESULT CChannel::Initialize(ifstream& inFile, const C3DModel* pModel)
{
	_uint iNameLength = 0;
	inFile.read(reinterpret_cast<char*>(&iNameLength), sizeof(_uint));
	if (iNameLength >= MAX_PATH)
		return E_FAIL;
	inFile.read(m_szName, iNameLength);
	m_szName[iNameLength] = '\0';
	m_iBoneIndex = pModel->Get_BoneIndex(m_szName);

	inFile.read(reinterpret_cast<char*>(&m_iNumKeyFrames), sizeof(_uint));

	for (_uint i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME		tKeyFrame = {};

		inFile.read(reinterpret_cast<char*>(&tKeyFrame), sizeof(KEYFRAME));

		m_KeyFrames.push_back(tKeyFrame);
	}
	return S_OK;
}

void CChannel::Update_TransformationMatrix(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones, _bool bReverse)
{
	if (0.f == fCurrentTrackPosition)
		*pKeyFrameIndex = 0;

	KEYFRAME tKeyFrame = Get_Frame(fCurrentTrackPosition, pKeyFrameIndex, bReverse);
	Bones[m_iBoneIndex]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&tKeyFrame.vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&tKeyFrame.vRotation), XMVectorSetW(XMLoadFloat3(&tKeyFrame.vPosition), 1.f)));

}


KEYFRAME CChannel::Get_Frame(_float fTrackPos, _uint* pOutCurrentKeyFrameIdx, _bool bReverse) const
{

	if (bReverse)
	{
		if (fTrackPos <= 0)
			return m_KeyFrames[0];
	}
	else if (fTrackPos >= m_KeyFrames.back().fTrackPosition)
		return m_KeyFrames.back();

	*pOutCurrentKeyFrameIdx = Get_KeyFrameIndex(fTrackPos, *pOutCurrentKeyFrameIdx, bReverse);
	if (bReverse)
	{
		if (*pOutCurrentKeyFrameIdx <= 0)
			return m_KeyFrames[*pOutCurrentKeyFrameIdx];
		_float fRatio = (m_KeyFrames[*pOutCurrentKeyFrameIdx].fTrackPosition - fTrackPos) /
			(m_KeyFrames[*pOutCurrentKeyFrameIdx].fTrackPosition - m_KeyFrames[*pOutCurrentKeyFrameIdx - 1].fTrackPosition);
		return Lerp_Frame(m_KeyFrames[*pOutCurrentKeyFrameIdx - 1],m_KeyFrames[*pOutCurrentKeyFrameIdx], 1 - fRatio);
	}
	else
	{
		if ((_int)*pOutCurrentKeyFrameIdx >= ((_int)m_KeyFrames.size() - 1))
			return m_KeyFrames[*pOutCurrentKeyFrameIdx];
		_float fRatio = (fTrackPos - m_KeyFrames[*pOutCurrentKeyFrameIdx].fTrackPosition) /
			(m_KeyFrames[*pOutCurrentKeyFrameIdx + 1].fTrackPosition - m_KeyFrames[*pOutCurrentKeyFrameIdx].fTrackPosition);
		return Lerp_Frame(m_KeyFrames[*pOutCurrentKeyFrameIdx], m_KeyFrames[*pOutCurrentKeyFrameIdx + 1], fRatio);;
	}

}

_uint CChannel::Get_KeyFrameIndex(_float fTrackPos, _uint iStartKeyFrameIdx, _bool bReverse) const
{
	if (0.f >= fTrackPos)
		return 0;
	if (fTrackPos >= m_KeyFrames.back().fTrackPosition)
		return (_uint)m_KeyFrames.size() - 1;

	if (bReverse)
	{
		if (iStartKeyFrameIdx > 0)
		{
			while (fTrackPos <= m_KeyFrames[iStartKeyFrameIdx - 1].fTrackPosition)
				--iStartKeyFrameIdx;
		}

	}
	else
	{
		if (iStartKeyFrameIdx < m_KeyFrames.size() - 1)
		{
			while (fTrackPos >= m_KeyFrames[iStartKeyFrameIdx + 1].fTrackPosition)
				++iStartKeyFrameIdx;
		}

	}

	return iStartKeyFrameIdx;
}


void CChannel::Get_Frame(_float fTrackPos, map<_uint, KEYFRAME>* pOutKeyFrames) const
{
	_uint iCurrentKeyFrameIdx = 0;
	(*pOutKeyFrames)[m_iBoneIndex] = Get_Frame(fTrackPos, &iCurrentKeyFrameIdx);
}


CChannel* CChannel::Create(ifstream& inFile, const C3DModel* pModel)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(inFile, pModel)))
	{
		MSG_BOX("Failed to Create : CChannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

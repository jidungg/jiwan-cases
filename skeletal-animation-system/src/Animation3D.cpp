#include "..\Public\Animation3D.h"

#include "Channel.h"
#include "Bone.h"

namespace Engine
{
	KEYFRAME Lerp_Frame(const KEYFRAME& Frame1, const KEYFRAME& Frame2, _float fRatio)
	{
		KEYFRAME KeyFrame = {};
		XMStoreFloat3(&KeyFrame.vScale, XMVectorLerp(XMLoadFloat3(&Frame1.vScale), XMLoadFloat3(&Frame2.vScale), fRatio));
		XMStoreFloat4(&KeyFrame.vRotation, XMQuaternionSlerp(XMLoadFloat4(&Frame1.vRotation), XMLoadFloat4(&Frame2.vRotation), fRatio));
		XMStoreFloat3(&KeyFrame.vPosition, XMVectorLerp(XMLoadFloat3(&Frame1.vPosition), XMLoadFloat3(&Frame2.vPosition), fRatio));

		return KeyFrame;
	}
}


HRESULT CAnimation3D::Initialize(ifstream& inFile, const C3DModel* pModel)
{
	_uint iNameLength = 0;
	inFile.read(reinterpret_cast<char*>(&iNameLength), sizeof(_uint));
	if (iNameLength >= MAX_PATH)
		return E_FAIL;
	inFile.read(m_szName, iNameLength);
	m_szName[iNameLength] = '\0';


	double dValue = 0.0;
	inFile.read(reinterpret_cast<char*>(&dValue), sizeof(double));
	m_fDuration = (_float)dValue;
	inFile.read(reinterpret_cast<char*>(&dValue), sizeof(double));
	m_fTickPerSecond = (_float)dValue;
	//Loop
	inFile.read(reinterpret_cast<char*>(&m_bLoop), sizeof(_bool));
	//SpeedMagnifier
	inFile.read(reinterpret_cast<char*>(&m_fSpeedMagnifier), sizeof(_float));


	inFile.read(reinterpret_cast<char*>(&m_iNumChannels), sizeof(_uint));

	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel* pChannel = CChannel::Create(inFile, pModel);
		if (nullptr == pChannel)
			return E_FAIL;

		m_vecChannel.push_back(pChannel);
	}
	return S_OK;
}

bool CAnimation3D::Update_TransformationMatrices(const vector<class CBone*>& Bones, _float fTimeDelta, _bool bReverse)
{

	if ((bReverse && m_fCurrentTrackPosition < 0)
		|| (false == bReverse && m_fCurrentTrackPosition > m_fDuration))
	{
		if (true == m_bLoop)
		{
			Reset(bReverse);
		}
		else
		{
			m_fCurrentTrackPosition = bReverse ? 0 : m_fDuration;
			return true;
		}
	}


	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		m_vecChannel[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[i], Bones, bReverse);
	}
	if (bReverse)
		m_fCurrentTrackPosition -= m_fTickPerSecond * fTimeDelta * m_fSpeedMagnifier;
	else
		m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta * m_fSpeedMagnifier;


	return false;
}

bool CAnimation3D::Update_AnimTransition(const vector<class CBone*>& Bones, _float fTimeDelta, const map<_uint, KEYFRAME>& mapAnimTransLeftFrame, _bool bReverse)
{
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta * m_fSpeedMagnifier;
	float _fAnimTransitionTrackPos = m_fAnimTransitionTime * m_fTickPerSecond;
	if (m_fCurrentTrackPosition > _fAnimTransitionTrackPos)
	{
		Reset(bReverse);
		return true;
	}

	_float			fRatio = m_fCurrentTrackPosition / _fAnimTransitionTrackPos;
	KEYFRAME tFrame;
	for (_uint channel = 0; channel < m_iNumChannels; channel++)
	{
		_uint iBoneIdx = m_vecChannel[channel]->Get_BoneIndex();
		KEYFRAME tFirstKey = m_vecChannel[channel]->Get_KeyFrame(0);
		auto iterLeft = mapAnimTransLeftFrame.find(iBoneIdx);
		const KEYFRAME& tLeftKey = (iterLeft == mapAnimTransLeftFrame.end()) ? tFirstKey : iterLeft->second;
		tFrame = Lerp_Frame(tLeftKey, tFirstKey, fRatio);
		Bones[iBoneIdx]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&tFrame.vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&tFrame.vRotation), XMVectorSetW(XMLoadFloat3(&tFrame.vPosition), 1.f)));
	}

	return false;
}

void CAnimation3D::Reset(_bool bReverse)
{
	__super::Reset(bReverse);
}


void CAnimation3D::Get_CurrentFrame(map<_uint,KEYFRAME>* pOutKeyFrames) const
{
	for (auto& channel : m_vecChannel)
		channel->Get_Frame(m_fCurrentTrackPosition, pOutKeyFrames);
}

_float CAnimation3D::Get_Progress()
{
	return m_fCurrentTrackPosition / m_fDuration;
}

_float CAnimation3D::Get_AnimationTime()
{
	return m_fDuration / m_fSpeedMagnifier / m_fTickPerSecond;
}

void CAnimation3D::Set_Progress(_float fProgress, _bool bReverse)
{
	m_fCurrentTrackPosition = fProgress * m_fDuration;
	Update_CurrentKeyFrameIndices(bReverse);
}

void CAnimation3D::Update_CurrentKeyFrameIndices(_bool bReverse)
{	//현재 TrackPosition이 되기 전의 키 프레임으로
	for (_uint i = 0; i < m_iNumChannels; i++)
	{
		m_CurrentKeyFrameIndices[i] = m_vecChannel[i]->Get_KeyFrameIndex(m_fCurrentTrackPosition, bReverse);
	}
}

CAnimation3D* CAnimation3D::Create(ifstream& inFile, const C3DModel* pModel)
{
	CAnimation3D* pInstance = new CAnimation3D();

	if (FAILED(pInstance->Initialize(inFile, pModel)))
	{
		MSG_BOX("Failed to Create : CAnimation3D");
		Safe_Release(pInstance);
	}
	return pInstance;
}


#include "stdafx.h"
#include "EffConcreteController.h"
#include "EffMaterialProperty.h"
#include "EffTexturingProperty.h"
#include "EffBone.h"
#include "EffModel.h"
#include "Engine_Utility.h"


#pragma region ALPHA

HRESULT CEffAlphaController::Initialize_Prototype(ifstream& inFile, const CEffModel* pModel)
{
	m_eCtrlType = EFF_CTRL_TYPE::CT_ALPHA;
	if (FAILED(__super::Initialize_Prototype(inFile, pModel)))
		return E_FAIL;
	_uint iNumFrame = 0;
	inFile.read(reinterpret_cast<char*>(&iNumFrame), sizeof(_uint));
	for (_uint i = 0; i < iNumFrame; i++)
	{
		FLOAT_KEYFRAME tNewFrame;
		inFile.read(reinterpret_cast<char*>(&tNewFrame.fValue), sizeof(_float));
		inFile.read(reinterpret_cast<char*>(&tNewFrame.fTrackPosition), sizeof(_float));
		m_vecKeyFrame.push_back(tNewFrame);
	}
	return S_OK;
}

_bool CEffAlphaController::Update_InTime(_float fTrackPos)
{
	assert(m_pTarget);

	CEffMaterialProperty* pTarget = static_cast<CEffMaterialProperty*>(m_pTarget);
	//GetCurrentKeyFrame : fTrackPos를 넘지 않는 가장 가까운 KeyFrame
	_uint iCurrentKeyFrameIndex = 0;
	_uint iNumFrame = (_uint)m_vecKeyFrame.size();
	if (iNumFrame == 1)
	{
		pTarget->Set_Alpha(m_vecKeyFrame[0].fValue);
		return true;
	}
	//마지막 프레임보다 많이 갔으면?
	if (m_vecKeyFrame[iNumFrame - 1].fTrackPosition < fTrackPos)
	{
		pTarget->Set_Alpha(m_vecKeyFrame[iNumFrame - 1].fValue);
		return true;
	}
	while (m_vecKeyFrame[iCurrentKeyFrameIndex + 1].fTrackPosition < fTrackPos)
	{
		iCurrentKeyFrameIndex++;

	}

	FLOAT_KEYFRAME tLeftKeyFrame = m_vecKeyFrame[iCurrentKeyFrameIndex];
	FLOAT_KEYFRAME tRightKeyFrame = m_vecKeyFrame[iCurrentKeyFrameIndex + 1];


	_float fRatio = (fTrackPos - tLeftKeyFrame.fTrackPosition)
		/ (tRightKeyFrame.fTrackPosition - tLeftKeyFrame.fTrackPosition);

	_float fAlpha = (tRightKeyFrame.fValue - tLeftKeyFrame.fValue) * fRatio + tLeftKeyFrame.fValue;

	pTarget->Set_Alpha(fAlpha);
	return false;
}

void CEffAlphaController::Set_Target(vector<CEffMaterialProperty*>& vecMaterial)
{
	m_pTarget = vecMaterial[m_iTargetIndex];
}

#pragma endregion

#pragma region TEXTURETRANSFORM

HRESULT CEffTextureTransformController::Initialize_Prototype(ifstream& inFile, const CEffModel* pModel)
{
	m_eCtrlType = EFF_CTRL_TYPE::CT_TEXTURE_TRANSFORM;
	if (FAILED(__super::Initialize_Prototype(inFile, pModel)))
		return E_FAIL;
	inFile.read(reinterpret_cast<char*>(&m_eTexSlot), sizeof(_uint));
	inFile.read(reinterpret_cast<char*>(&m_eTexOperation), sizeof(_uint));
	_uint iNumFrame = 0;
	inFile.read(reinterpret_cast<char*>(&iNumFrame), sizeof(_uint));
	for (_uint i = 0; i < iNumFrame; i++)
	{
		FLOAT_KEYFRAME tNewFrame;
		inFile.read(reinterpret_cast<char*>(&tNewFrame.fValue), sizeof(_float));
		inFile.read(reinterpret_cast<char*>(&tNewFrame.fTrackPosition), sizeof(_float));
		m_vecKeyFrame.push_back(tNewFrame);
	}
	return S_OK;
}

_bool CEffTextureTransformController::Update_InTime(_float fTrackPos)
{
	assert(m_pTarget);
	CEffTexturingProperty* pTarget = static_cast<CEffTexturingProperty*>(m_pTarget);
	//GetCurrentKeyFrame : fTrackPos를 넘지 않는 가장 가까운 KeyFrame
	_uint iCurrentKeyFrameIndex = 0;
	_uint iNumFrame = (_uint)m_vecKeyFrame.size();
	if (iNumFrame == 1)
	{
		pTarget->Set_TextureTransformData(m_eTexSlot, m_eTexOperation, m_vecKeyFrame[0].fValue);
		return true;
	}
	if (m_vecKeyFrame[iNumFrame - 1].fTrackPosition < fTrackPos)
	{
		pTarget->Set_TextureTransformData(m_eTexSlot, m_eTexOperation, m_vecKeyFrame[iNumFrame - 1].fValue);
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
	pTarget->Set_TextureTransformData(m_eTexSlot, m_eTexOperation, fValue);
	return false;
}

void CEffTextureTransformController::Set_Target(vector<CEffTexturingProperty*>& vecTexturing)
{
	m_pTarget = vecTexturing[m_iTargetIndex];
}


#pragma endregion

#pragma region TRANSFORM

HRESULT CEffTransformController::Initialize_Prototype(ifstream& inFile, const CEffModel* pModel)
{
	m_eCtrlType = EFF_CTRL_TYPE::CT_TRANSFORM;
	if (FAILED(__super::Initialize_Prototype(inFile, pModel)))
		return E_FAIL;
	_uint iNumFrame = 0;
	inFile.read(reinterpret_cast<char*>(&iNumFrame), sizeof(_uint));
	for (_uint i = 0; i < iNumFrame; i++)
	{
		TRANSFORM_KEYFRAME tNewFrame;
		inFile.read(reinterpret_cast<char*>(&tNewFrame.vScale), sizeof(_float3));


		inFile.read(reinterpret_cast<char*>(&tNewFrame.vRotation), sizeof(_float3));

		inFile.read(reinterpret_cast<char*>(&tNewFrame.vPosition), sizeof(_float3));

		inFile.read(reinterpret_cast<char*>(&tNewFrame.fTrackPosition), sizeof(_float));
		m_vecKeyFrame.push_back(tNewFrame);
	}
	inFile.read(reinterpret_cast<char*>(&m_sFlags), sizeof(_ushort));

	return S_OK;
}


_bool CEffTransformController::Update_InTime(_float fTrackPos)
{
	assert(m_pTarget);

	CEffBone* pTarget = static_cast<CEffBone*>(m_pTarget);
	//GetCurrentKeyFrame : fTrackPos를 넘지 않는 가장 가까운 KeyFrame
	_uint iCurrentKeyFrameIndex = 0;
	_uint iNumFrame = (_uint)m_vecKeyFrame.size();
	if (iNumFrame == 0)
		return true;

	TRANSFORM_KEYFRAME tKeyFrame;
	_bool bRet = false;
	if (iNumFrame == 1)
	{
		tKeyFrame = m_vecKeyFrame[0];
		bRet = true;
	}
	else if (m_vecKeyFrame[iNumFrame - 1].fTrackPosition < fTrackPos)
	{
		tKeyFrame = m_vecKeyFrame[iNumFrame - 1];
		bRet = true;
	}
	else
	{
		while (m_vecKeyFrame[iCurrentKeyFrameIndex + 1].fTrackPosition < fTrackPos)
			iCurrentKeyFrameIndex++;

		TRANSFORM_KEYFRAME tLeftKeyFrame = m_vecKeyFrame[iCurrentKeyFrameIndex];
		TRANSFORM_KEYFRAME tRightKeyFrame = m_vecKeyFrame[iCurrentKeyFrameIndex + 1];

		_float fRatio = (fTrackPos - tLeftKeyFrame.fTrackPosition)
			/ (tRightKeyFrame.fTrackPosition - tLeftKeyFrame.fTrackPosition);
		tKeyFrame = CEngineUtility::Lerp_NonQuaternionFrame(tLeftKeyFrame, tRightKeyFrame, fRatio);
		bRet = false;
	}

	_matrix mat = XMMatrixIdentity();
	mat *= XMMatrixScalingFromVector(XMLoadFloat3(&tKeyFrame.vScale));
	mat *= XMMatrixRotationAxis({ 1.f,0.f,0.f }, tKeyFrame.vRotation.x);
	mat *= XMMatrixRotationAxis({ 0.f,1.f,0.f }, tKeyFrame.vRotation.y);
	mat *= XMMatrixRotationAxis({ 0.f,0.f,1.f }, tKeyFrame.vRotation.z);
	mat *= XMMatrixTranslationFromVector(XMLoadFloat3(&tKeyFrame.vPosition));

	pTarget->Set_TransformationMatrix(mat);
	return bRet;
}

void CEffTransformController::Set_Target(vector<CEffBone*>& vecBone)
{
	m_pTarget = vecBone[m_iTargetIndex];
}

#pragma endregion

#pragma region MATERIALCOLOR

HRESULT CEffMaterialColorController::Initialize_Prototype(ifstream& inFile, const CEffModel* pModel)
{
	m_eCtrlType = EFF_CTRL_TYPE::CT_MATERIAL_COLOR;
	if (FAILED(__super::Initialize_Prototype(inFile, pModel)))
		return E_FAIL;
	return S_OK;
}
_bool CEffMaterialColorController::Update_InTime(_float fTrackPos)
{
	assert(m_pTarget);
	return true;
}

void CEffMaterialColorController::Set_Target(vector<CEffMaterialProperty*>& vecMaterial)
{
	m_pTarget = vecMaterial[m_iTargetIndex];
}

#pragma endregion

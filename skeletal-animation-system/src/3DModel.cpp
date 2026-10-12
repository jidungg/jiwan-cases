#include "3DModel.h"
#include "Mesh.h"
#include "Bone.h"
#include "Animation3D.h"

HRESULT C3DModel::Bind_Matrices(CShader* _pShader, const _char* _pConstantName, _uint _iMeshIndex)
{
	/* 모델의 m_Bones 벡터를 넘겨 여기서 자기들이 필요한 메쉬에 접근해서 행렬을 가져오고 연산할 것임. */
	return m_Meshes[_iMeshIndex]->Bind_BoneMatrices(_pShader, _pConstantName, m_Bones);
}

_bool C3DModel::Play_Animation(_float _fTimeDelta, _bool _bReverse)
{
	m_bReverseAnimation = _bReverse;
	_bool bAnimEnd = false;

	//뼈들의 변환행렬을 갱신
	//애니메이션 전환이 아닐 때
	if (m_iCurrentAnimIndex == m_iPrevAnimIndex)
	{
		bAnimEnd = m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrices(m_Bones, _fTimeDelta, _bReverse);
		m_bDuringAnimation = (false == bAnimEnd);
	}
	//애니메이션 전환 중
	else
	{
		if (m_Animations[m_iCurrentAnimIndex]->Update_AnimTransition(m_Bones, _fTimeDelta, m_mapAnimTransLeftFrame, _bReverse))
			m_iPrevAnimIndex = m_iCurrentAnimIndex;
		m_bDuringAnimation = false;
		bAnimEnd = false;
	}


	//뼈들의 합성변환행렬을 갱신
	_matrix PreTransformMatrix = XMLoadFloat4x4(&m_PreTransformMatrix);
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(m_Bones, PreTransformMatrix);

	return bAnimEnd;
}

_uint C3DModel::Get_BoneIndex(const _char* _pBoneName) const
{

	_uint	iBoneIndex = { 0 };

	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
		{
			if (false == strcmp(pBone->Get_Name(), _pBoneName))
				return true;

			++iBoneIndex;

			return false;
		});

	if (iter == m_Bones.end())
		MSG_BOX("그런 뼈가 없어");

	return iBoneIndex;
}

void C3DModel::Set_AnimationLoop(_uint _iIdx, _bool _bIsLoop)
{
	_int iTemp = (_int)m_Animations.size() - 1;
	if (iTemp < (_int)_iIdx)
	{
		return;
	}
	m_Animations[_iIdx]->Set_Loop(_bIsLoop);
}

void C3DModel::Set_AnimationTransitionTime(_uint _iIdx, _float _fTime)
{
	_int iTemp = (_int)m_Animations.size() - 1;
	if (iTemp < (_int)_iIdx)
	{
		return;
	}
	m_Animations[_iIdx]->Set_AnimTransitionTime(_fTime);
}

void C3DModel::Set_Progress(_uint _iIdx, _float _fProgress, _bool _bReverse)
{
	_int iSize = m_Animations.size() - 1;
	if (iSize < (_int)_iIdx)
	{
		return;
	}
	m_Animations[_iIdx]->Set_Progress(_fProgress, _bReverse);
}


void C3DModel::Switch_Animation(_uint _iIdx, _bool _bReverse)
{
	_int iTemp = (_int)m_Animations.size() - 1;
	if (iTemp < (_int)_iIdx)
	{
		return;
	}
	m_iPrevAnimIndex = m_iCurrentAnimIndex;
	m_iCurrentAnimIndex = _iIdx;
	m_mapAnimTransLeftFrame.clear();
	m_Animations[m_iCurrentAnimIndex]->Reset(_bReverse);
	m_Animations[m_iPrevAnimIndex]->Get_CurrentFrame(&m_mapAnimTransLeftFrame);
	m_mapAnimTransLeftFrame[1].vPosition.x = 0.f;
	m_mapAnimTransLeftFrame[1].vPosition.z = 0.f;

}

void C3DModel::Switch_Reverse(_uint _iIdx, _bool _bReverse)
{
	m_Animations[_iIdx]->Update_CurrentKeyFrameIndices(_bReverse);
}

void C3DModel::Set_AnimSpeedMagnifier(_uint _iAnimIndex, _float _fMag)
{
	if (_iAnimIndex >= m_Animations.size())
	{
		return;
	}
	m_Animations[_iAnimIndex]->Set_SpeedMagnifier(_fMag);
}

HRESULT C3DModel::Ready_Bones(ifstream& _inFile, _uint _iParentBoneIndex)
{

	CBone* pBone = CBone::Create(_inFile, _iParentBoneIndex);;


	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	_iParentBoneIndex = (_uint)m_Bones.size() - 1;
	_uint iNumChildren = 0;
	_inFile.read(reinterpret_cast<char*>(&iNumChildren), sizeof(_uint));
	for (_uint i = 0; i < iNumChildren; ++i)
	{
		Ready_Bones(_inFile, _iParentBoneIndex);
	}

	return S_OK;
}

HRESULT C3DModel::Ready_Animations(ifstream& _inFile)
{
	_inFile.read(reinterpret_cast<char*>(&m_iNumAnimations), sizeof(_uint));
	m_Animations.reserve(m_iNumAnimations);
	for (_uint i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation3D* pAnimation = CAnimation3D::Create(_inFile, this);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}

	return S_OK;
}

_float C3DModel::Get_CurrentAnimProgeress()
{
	return m_Animations[m_iCurrentAnimIndex]->Get_Progress();
}


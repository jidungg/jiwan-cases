#pragma once
#include "Model.h"

BEGIN(Engine)
class CMesh;
class CBone;
class CAnimation3D;
class CShader;
class ENGINE_DLL C3DModel : public CModel
{
public:
	HRESULT					Bind_Matrices(CShader* _pShader, const _char* _pConstantName, _uint _iMeshIndex);
	virtual _bool			Play_Animation(_float _fTimeDelta, _bool _bReverse = false) override;

public:
	// Get
	_uint					Get_BoneIndex(const _char* _pBoneName) const;
	virtual _uint			Get_CurrentAnimIndex() override { return m_iCurrentAnimIndex; }
	virtual _float			Get_CurrentAnimProgeress() override;
	// Set
	virtual void			Set_AnimationLoop(_uint _iIdx, _bool _bIsLoop)override;
	void					Set_AnimationTransitionTime(_uint _iIdx, _float _fTime);
	virtual void			Set_Progress(_uint _iIdx, _float _fProgress, _bool _bReverse) override;
	virtual void			Switch_Animation(_uint _iIdx, _bool _bReverse = false)override;
	virtual void			Switch_Reverse(_uint _iIdx, _bool _bReverse = false) override;
	virtual void			Set_AnimSpeedMagnifier(_uint _iAnimIndex, _float _fMag) override;

protected:
	vector<CMesh*>				m_Meshes;

	vector<CBone*>				m_Bones;
	_float4x4					m_PreTransformMatrix = {};

	_uint						m_iCurrentAnimIndex = {};
	_uint						m_iPrevAnimIndex = {};
	_uint						m_iNumAnimations = 0;
	vector<CAnimation3D*>		m_Animations;


	map<_uint, KEYFRAME>		m_mapAnimTransLeftFrame;

protected:
	virtual HRESULT Ready_Bones(ifstream& _inFile, _uint _iParentBoneIndex);
	virtual HRESULT Ready_Animations(ifstream& _inFile);
};

END

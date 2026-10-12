#pragma once
#include "EffController.h"
BEGIN(Client)
class CEffBone;
class CEffTexturingProperty;
class CEffMaterialProperty;

//=====================================================ALPHA ================================================================
class CEffAlphaController :
	public CEffController
{
public:
	virtual HRESULT Initialize_Prototype(ifstream& inFile, const class CEffModel* pModel) override;
	virtual _bool Update_InTime(_float fTrackPos) override;
	void Set_Target(vector<CEffMaterialProperty*>& vecMaterial);
private:
	vector<FLOAT_KEYFRAME> m_vecKeyFrame;
};

//=====================================================TEXTURE TRANSFORM=======================================================
class CEffTextureTransformController :
	public CEffController
{
public:
	virtual HRESULT Initialize_Prototype(ifstream& inFile, const class CEffModel* pModel) override;
	virtual _bool Update_InTime(_float fTrackPos) override;
	void Set_Target(vector<CEffTexturingProperty*>& vecTexturing);
private:
	EFF_TEX_TYPE m_eTexSlot = EFF_TEX_TYPE::TT_LAST;
	EFF_TEX_OPERATION_TYPE m_eTexOperation = EFF_TEX_OPERATION_TYPE::TO_LAST;
	vector<FLOAT_KEYFRAME> m_vecKeyFrame;
};


//=====================================================TRANSFORM=======================================================
class CEffTransformController :
	public CEffController
{
public:
	virtual HRESULT Initialize_Prototype(ifstream& inFile, const class CEffModel* pModel) override;
	virtual _bool Update_InTime(_float fTrackPos) override;

	void Set_Target(vector<CEffBone*>& vecBone);
private:
	vector<TRANSFORM_KEYFRAME> m_vecKeyFrame;
	_ushort m_sFlags = { 0 };
};


//=====================================================MATERIAL COLOR =======================================================
class CEffMaterialColorController :
	public CEffController
{
public:
	virtual HRESULT Initialize_Prototype(ifstream& inFile, const class CEffModel* pModel) override;

	virtual _bool Update_InTime(_float fTrackPos) override;
	void Set_Target(vector<CEffMaterialProperty*>& vecMaterial);
};
END
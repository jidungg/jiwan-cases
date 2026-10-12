#pragma once
#include "EffController.h"

BEGIN(Client)
class CEffTexturingProperty;
class CEffFlipController : public CEffController
{
public:
	virtual HRESULT Initialize_Prototype(ifstream& inFile, const class CEffModel* pModel) override;
	virtual _bool Update_InTime(_float fTrackPos) override;
	void Set_Target(vector<CEffTexturingProperty*>& vecTexturing);
private:
	EFF_TEX_TYPE m_eTexSlot = EFF_TEX_TYPE::TT_LAST;
	vector<_uint> vecSourceTextureIndex;
	vector<FLOAT_KEYFRAME> m_vecKeyFrame;
};

END
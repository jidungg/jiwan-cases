#pragma once
#include "Animation.h"
#include "Base.h"

BEGIN(Engine)
KEYFRAME ENGINE_DLL Lerp_Frame(const KEYFRAME& Frame1, const KEYFRAME& Frame2, _float fRatio);

class ENGINE_DLL CAnimation3D : public CAnimation
{
public:
	virtual HRESULT Initialize(ifstream& inFile, const class C3DModel* pModel);
	bool Update_TransformationMatrices(const vector<class CBone*>& Bones, _float fTimeDelta, _bool bReverse = false);
	bool Update_AnimTransition(const vector<class CBone*>& Bones, _float fTimeDelta, const map<_uint, KEYFRAME>& mapAnimTransLeftFrame, _bool bReverse = false);
	void Update_CurrentKeyFrameIndices(_bool bReverse);

	virtual void Reset(_bool bReverse = false)override;

	_float Get_AnimTransitionTime() { return m_fAnimTransitionTime; }
	void Get_CurrentFrame(map<_uint, KEYFRAME>* pOutKeyFrames) const;
	virtual _float Get_Progress() override;
	virtual _float Get_AnimationTime() override;

	virtual void Set_Progress(_float fProgress, _bool bReverse)override;
	void Set_AnimTransitionTime(_float fTime) { m_fAnimTransitionTime = fTime; }
protected:
	_char					m_szName[MAX_PATH] = {};
	_uint					m_iNumChannels = {};
	vector<class CChannel*> m_vecChannel;


	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};
	_float					m_fAnimTransitionTime = { 0.1f };

	//Channel마다 현재 KeyFrameIndex를 저장하는 배열
	vector<_uint>			m_CurrentKeyFrameIndices;

public:
	static CAnimation3D* Create(ifstream& inFile, const class C3DModel* pModel);
};

END
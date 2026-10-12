#pragma once

#include "Base.h"


BEGIN(Engine)

class ENGINE_DLL CChannel : public CBase
{
public:
	HRESULT Initialize(ifstream& inFile, const class C3DModel* pModel);
	void Update_TransformationMatrix(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones, _bool bReverse);

	void Get_Frame(_float fTrackPos, map<_uint, KEYFRAME>* pOutKeyFrames) const;
	KEYFRAME Get_Frame(_float fTrackPos, _uint* pOutCurrentKeyFrameIdx, _bool bReverse = false) const;
	_uint Get_KeyFrameIndex(_float fTrackPos, _uint iStartKeyFrameIdx = 0, _bool bReverse = false) const;
	_uint Get_BoneIndex() const { return m_iBoneIndex; }
	KEYFRAME Get_KeyFrame(_uint iKeyFrameIndex) { return m_KeyFrames[iKeyFrameIndex]; }
protected:
	_char						m_szName[MAX_PATH] = {};
	_uint						m_iNumKeyFrames = {};
	vector<KEYFRAME>			m_KeyFrames;
	_uint						m_iBoneIndex = {};

public:
	static CChannel* Create(ifstream& inFile, const class C3DModel* pModel);
};

END
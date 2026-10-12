#pragma once
#include "Level.h"
#include "AnimEventGenerator.h"

BEGIN(AnimTool)
class CTestModelObject;

class CLevel_AnimTool :
	public CLevel
{
public:
	void Update_AnimEventImgui();

private:
	HRESULT					Load_AnimEvents(wstring _wstrPath);
	HRESULT					Export_AnimEvents(const wstring& _wstrPath);

private:
	CTestModelObject* m_pTestModelObj = nullptr;

	_int m_iCurrentAnimIndex = 0;
	_float m_fCurrentProgress = 0;

	map<_uint, vector<ANIM_EVENT>> m_AnimEvents;
	_int m_iSelectedAnimEventIndex = -1;

	static constexpr _uint m_iFuncNameBufferSize = 128;
};

END
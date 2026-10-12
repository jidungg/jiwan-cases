#pragma once
#include "Base.h"
/* 2D Model(CVIBuffer_Rect) 와 3D Model(CModel) 을 관리하는 클래스. */
/* 현재 Object의 Coordinate State에 따라, 기능을 실행, 제한 등 컨트롤을 하는 클래스 */
/* Transform에 비해 상대적으로 공통 된 기능이 없다시피 함. PlayAnimation, SetupAnimation 정도? */

BEGIN(Engine)
class CModel;
class ENGINE_DLL CController_Model : public CBase
{
public:
	HRESULT			Change_Coordinate(COORDINATE _eCoordinate);


public: /* 2D, 3D */
	void			Play_Animation(_float _fTimeDelta, _bool _bReverse = false);
	void			Register_OnAnimEndCallBack(const function<void(COORDINATE, _uint)>& _fCallback) { m_listAnimEndCallBack.push_back(_fCallback); }
	void			Invoke_OnAnimEndCallBack(COORDINATE _eCoord, _uint _iAnimIdx);
	//Set
	void			Set_AnimationLoop(COORDINATE _eCoord, _uint _iIdx, _bool _bIsLoop);
	void			Switch_Animation(_uint _iIdx, _bool _bReverse = false);

protected:

	CModel* m_ModelComs[COORDINATE_LAST] = {};
	COORDINATE				m_eCurCoord = COORDINATE_LAST;


	list<function<void(COORDINATE, _uint)>> m_listAnimEndCallBack;
};
END


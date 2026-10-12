#pragma once
#include "Component.h"
#include "PlayerState.h"

BEGIN(Client)
class CStateMachine :
	public CComponent
{
public:
	void Update(_float fTimeDelta);

	void Register_OnStateChangeCallBack(const function<void(CPlayerState*)>& fCallback) { m_listStateChangeCallback.push_back(fCallback); }
	
	//Get
	CPlayerState* Get_CurrentState() { return m_pState; }
	//Set
	void Transition_To(CPlayerState* pNewState);

private:
	CPlayerState* m_pState = nullptr;

	list<function<void(CPlayerState*)>> m_listStateChangeCallback;
};

END
#include "stdafx.h"
#include "StateMachine.h"

void CStateMachine::Update(_float fTimeDelta)
{
	Get_CurrentState()->Update(fTimeDelta);
}

void CStateMachine::Transition_To(CPlayerState* pNewState)
{
	if (nullptr != m_pState)
	{
		m_pState->Exit();
		Safe_Release(m_pState);
	}
	m_pState = pNewState;
	m_pState->Enter();
	for (auto& callback : m_listStateChangeCallback)
		callback(pNewState);
}
#include "stdafx.h"
#include "PlayerState.h"
#include "GameInstance.h"

CPlayerState::CPlayerState(CPlayer* _pOwner, CPlayer::STATE _eState)
	:CBase()
	, m_eStateID(_eState)
	, m_pOwner(_pOwner)
	, m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

void Client::CPlayerState::Free()
{
	Safe_Release(m_pGameInstance);
	__super::Free();

}

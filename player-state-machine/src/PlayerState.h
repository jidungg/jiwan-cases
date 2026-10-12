#pragma once

#include "Player.h"

BEGIN(Engine)
class CGameInstance;

END
BEGIN(Client)
class CPlayerState : public CBase
{
protected:
	CPlayerState(CPlayer* _pOwner, CPlayer::STATE _eState);

	virtual ~CPlayerState() = default;
public:
	virtual void Update(_float _fTimeDelta) abstract;
	virtual void Enter() {};
	virtual void Exit() {};
	virtual void On_AnimEnd(COORDINATE _eCoord, _uint _iAnimIdx) {};

protected:
	CPlayer::STATE m_eStateID;
	CGameInstance* m_pGameInstance = nullptr;
	CPlayer* m_pOwner = nullptr;
public:
	void Free() override;
};
END
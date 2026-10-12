#include "stdafx.h"
#include "PlayerState_Idle.h"
#include "GameInstance.h"
#include "Player.h"
#include "Interactable.h"

namespace
{
	constexpr _double dFallingUpForceThreshold = -0.001;
}

void CPlayerState_Idle::Update(_float _fTimeDelta)
{

	COORDINATE eCoord = m_pOwner->Get_CurCoord();
	PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
	if (COORDINATE_3D == eCoord)
	{
		_float fUpForce = m_pOwner->Get_UpForce();
		_bool bOnGround = m_pOwner->Is_OnGround();
		if (false == bOnGround && dFallingUpForceThreshold > fUpForce)
		{
			m_pOwner->Set_State(CPlayer::JUMP_DOWN);
			return;
		}


		_bool bSneak = tKeyResult.bInputStates[PLAYER_INPUT_SNEAK];
		if (bSneak != m_bSneakBefore)
		{
			Switch_IdleAnimation3D(bSneak);
			m_bSneakBefore = bSneak;
		}
		
	}

	//Interact 시도 후 실패하면 나머지 입력 받기.
	INTERACT_RESULT eResult = m_pOwner->Try_Interact(_fTimeDelta);
	if (INTERACT_RESULT::FAIL == eResult
		|| INTERACT_RESULT::NO_INPUT == eResult)
	{

		if (tKeyResult.bInputStates[PLAYER_INPUT_ATTACK])
			m_pOwner->Set_State(CPlayer::ATTACK);
		else if (tKeyResult.bInputStates[PLAYER_INPUT_SPINATTACK])
			m_pOwner->Set_State(CPlayer::SPINATTACK);
		else if (tKeyResult.bInputStates[PLAYER_INPUT_JUMP])
			m_pOwner->Set_State(CPlayer::JUMP_UP);
		else if (tKeyResult.bInputStates[PLAYER_INPUT_ROLL])
			m_pOwner->Set_State(CPlayer::ROLL);
		else if (tKeyResult.bInputStates[PLAYER_INPUT_THROWSWORD])
			m_pOwner->Set_State(CPlayer::THROWSWORD);
		else if (tKeyResult.bInputStates[PLAYER_INPUT_MOVE])
			m_pOwner->Set_State(CPlayer::RUN);
		else if (tKeyResult.bInputStates[PLAYER_INPUT_THROWOBJECT])
			m_pOwner->Set_State(CPlayer::LAYDOWNOBJECT);
	}


}

void CPlayerState_Idle::Enter()
{
	COORDINATE eCoord = m_pOwner->Get_CurCoord();
	if (COORDINATE_2D == eCoord)
	{
		m_bPlatformerMode = m_pOwner->Is_PlatformerMode();

		F_DIRECTION eFDir = To_FDirection(m_pOwner->Get_2DDirection());
		if (F_DIRECTION::F_DIR_LAST == eFDir)
		{
			eFDir = F_DIRECTION::DOWN;
			m_pOwner->Set_2DDirection(eFDir);
		}
		Switch_IdleAnimation2D(eFDir);
	}
	else
	{
		m_pOwner->Stop_Move();
		PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
		Switch_IdleAnimation3D(tKeyResult.bInputStates[PLAYER_INPUT_SNEAK]);
	}
}

void CPlayerState_Idle::Exit()
{
}

void CPlayerState_Idle::Switch_IdleAnimation2D(F_DIRECTION _eFDir)
{
	if (m_pOwner->Is_CyvberJotMode())
	{
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_CYBERJOTLITE_RUN_IDLE_TEMP);
		return;
	}
	_bool bSword = m_pOwner->Is_SwordHandling();
	_bool bCarrying = m_pOwner->Is_CarryingObject();
	_bool bCyber = m_pOwner->Is_CyvberJotMode();
	if (m_bPlatformerMode)
	{
		if (F_DIRECTION::UP == _eFDir)
			_eFDir = F_DIRECTION::RIGHT;
		else if (F_DIRECTION::DOWN == _eFDir)
			_eFDir = F_DIRECTION::LEFT;
	}

	switch (_eFDir)
	{
	case Client::F_DIRECTION::LEFT:
	case Client::F_DIRECTION::RIGHT:
		if (bCarrying)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_CARRY_IDLE_RIGHT);
		else if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_IDLE_SWORD_RIGHT);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_IDLE_RIGHT);
		break;
	case Client::F_DIRECTION::UP:
		if (bCarrying)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_CARRY_IDLE_UP);
		else if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_IDLE_SWORD_UP);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_IDLE_UP);
		break;
	case Client::F_DIRECTION::DOWN:
		if (bCarrying)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_CARRY_IDLE_DOWN);
		else if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_IDLE_SWORD_DOWN);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_IDLE_DOWN);
		break;
	case Client::F_DIRECTION::F_DIR_LAST:
	default:
		break;
	}
}

void CPlayerState_Idle::Switch_IdleAnimation3D(_bool _bStealth)
{
	if (_bStealth)
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_STEALTH_IDLE_GT);
	else if (m_pOwner->Is_CarryingObject())
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_PICKUP_IDLE_GT);
	else if (m_pOwner->Is_DetonationMode())
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_REMOTE_HOLD);
	else
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_IDLE_01_GT);

}

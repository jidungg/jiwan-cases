#include "stdafx.h"
#include "PlayerState_Run.h"
#include "GameInstance.h"
#include "Player.h"

namespace
{
	constexpr _float fFallingUpForceThreshold = -0.01f;
	constexpr _float fSneakSpeedRatio = 0.5f;
	constexpr _float fFootstepSfxInterval = 0.3f;
	constexpr _int iFootstepSfxCount = 20;
	constexpr _float fFootstepSfxVolume = 20.f;
}

void CPlayerState_Run::Update(_float _fTimeDelta)
{
	_float fUpForce = m_pOwner->Get_UpForce();
	_bool bOnGround = m_pOwner->Is_OnGround();
	COORDINATE eCoord = m_pOwner->Get_CurCoord();
	if (COORDINATE_3D == eCoord)
	{
		_float fUpForce = m_pOwner->Get_UpForce();
		_bool bOnGround = m_pOwner->Is_OnGround();
		if (false == bOnGround && fFallingUpForceThreshold > fUpForce)
		{
			m_pOwner->Set_State(CPlayer::JUMP_DOWN);
			return;
		}
	}
	INTERACT_RESULT eResult = m_pOwner->Try_Interact(_fTimeDelta);
	if (INTERACT_RESULT::FAIL == eResult
		|| INTERACT_RESULT::NO_INPUT == eResult)
	{
		PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
		_bool bSneak = tKeyResult.bInputStates[PLAYER_INPUT_SNEAK];
		if (tKeyResult.bInputStates[PLAYER_INPUT_MOVE])
		{
			_float fMoveSpeed = m_pOwner->Get_MoveSpeed(eCoord);;
			COORDINATE eCoord = m_pOwner->Get_CurCoord();
			if (COORDINATE_2D == eCoord)
			{
				E_DIRECTION eNewDir = To_EDirection(tKeyResult.vMoveDir);
				F_DIRECTION eFDir = To_FDirection(eNewDir);
				m_pOwner->Set_2DDirection(To_EDirection(tKeyResult.vDir));

				if (m_eOldFDir != eFDir)
				{
					Switch_RunAnimation2D(eFDir);
					m_eOldFDir = eFDir;
				}
			}
			else
			{
				m_pOwner->Rotate_To(XMVector3Normalize(tKeyResult.vMoveDir), m_fRotateSpeed);
				if (bSneak != m_bSneakBefore)
				{
					Switch_RunAnimation3D(bSneak);
					m_bSneakBefore = bSneak;
				}
				fMoveSpeed = bSneak ? fMoveSpeed * fSneakSpeedRatio : fMoveSpeed;
			}
			m_pOwner->Move(XMVector3Normalize(tKeyResult.vMoveDir) * fMoveSpeed, _fTimeDelta);

			// 걷는 사운드 재생
			m_fAccSoundDelay += _fTimeDelta;
			if (fFootstepSfxInterval <= m_fAccSoundDelay)
			{
				if (false == bSneak)
					m_pGameInstance->Start_SFX(_wstring(L"A_sfx_footsteps_generic-") + to_wstring(rand() % iFootstepSfxCount), fFootstepSfxVolume);
				else
					m_pGameInstance->Start_SFX(_wstring(L"A_sfx_footsteps_sneak-") + to_wstring(rand() % iFootstepSfxCount), fFootstepSfxVolume);

				m_fAccSoundDelay = 0.f;
			}

			if (tKeyResult.bInputStates[PLAYER_INPUT_ATTACK])
				m_pOwner->Set_State(CPlayer::ATTACK);
			else if (tKeyResult.bInputStates[PLAYER_INPUT_SPINATTACK])
				m_pOwner->Set_State(CPlayer::SPINATTACK);
			else if (tKeyResult.bInputStates[PLAYER_INPUT_START_STAMP])
				m_pOwner->Set_State(CPlayer::STAMP);
			else if (tKeyResult.bInputStates[PLAYER_INPUT_JUMP])
				m_pOwner->Set_State(CPlayer::JUMP_UP);
			else if (tKeyResult.bInputStates[PLAYER_INPUT_ROLL])
				m_pOwner->Set_State(CPlayer::ROLL);
			else if (tKeyResult.bInputStates[PLAYER_INPUT_THROWSWORD])
				m_pOwner->Set_State(CPlayer::THROWSWORD);
			else if (tKeyResult.bInputStates[PLAYER_INPUT_THROWOBJECT])
				m_pOwner->Set_State(CPlayer::THROWOBJECT);
			return;
		}
		else
		{
			m_pOwner->Set_State(CPlayer::IDLE);
			return;
		}

	}
		
}

void CPlayerState_Run::Enter()
{
	COORDINATE eCoord = m_pOwner->Get_CurCoord();

	PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
	m_bSneakBefore = tKeyResult.bInputStates[PLAYER_INPUT_SNEAK];
	if (COORDINATE_2D == eCoord)
	{
		m_bPlatformerMode = m_pOwner->Is_PlatformerMode();
	}
	else
	{
		PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
		Switch_RunAnimation3D(tKeyResult.bInputStates[PLAYER_INPUT_SNEAK]);
	}

	m_pGameInstance->Start_SFX(_wstring(L"A_sfx_footsteps_generic-") + to_wstring(rand() % iFootstepSfxCount), fFootstepSfxVolume);
}

void CPlayerState_Run::Exit()
{
	COORDINATE eCoord = m_pOwner->Get_CurCoord();

	if (COORDINATE_2D == eCoord)
	{
	}
	else
	{
		m_pOwner->Stop_Rotate();
	}

	m_fAccSoundDelay = 0.f;

}

void CPlayerState_Run::Switch_RunAnimation2D(F_DIRECTION _eFDir)
{
	if (m_pOwner->Is_CyvberJotMode())
	{
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_CYBERJOTLITE_RUN_RIGHT);
		return;
	}
	_bool bSword = m_pOwner->Is_SwordHandling();
	_bool bCarrying = m_pOwner->Is_CarryingObject();
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
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_OBJECT_RIGHT);
		else if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_SWORD_RIGHT);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_RIGHT);
		break;
	case Client::F_DIRECTION::UP:
		if (bCarrying)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_OBJECT_UP);
		else if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_SWORD_UP);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_UP);
		break;
	case Client::F_DIRECTION::DOWN:
		if (bCarrying)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_OBJECT_DOWN);
		else if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_SWORD_DOWN);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_RUN_DOWN);
		break;
	case Client::F_DIRECTION::F_DIR_LAST:
	default:
		break;
	}
}

void CPlayerState_Run::Switch_RunAnimation3D(_bool _bStealth)
{
	if (_bStealth)
		m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_STEALTH_RUN_GT);
	else
	{
		_bool bSword = m_pOwner->Is_SwordHandling();
		_bool bCarrying = m_pOwner->Is_CarryingObject();
		if (bSword)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_RUN_SWORD_01_GT);
		else if (bCarrying)
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_RUN_01_GT);
		else
			m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_RUN_01_GT);
	}
}

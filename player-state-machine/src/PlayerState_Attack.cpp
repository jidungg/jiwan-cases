#include "stdafx.h"
#include "PlayerState_Attack.h"
#include "GameInstance.h"
#include "Player.h"

void CPlayerState_Attack::Update(_float _fTimeDelta)
{
    m_pOwner->Stop_Rotate();
    PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
    if (tKeyResult.bInputStates[PLAYER_INPUT_ATTACK])
    {
        m_bCombo = true;
    }
    COORDINATE eCoord = m_pOwner->Get_CurCoord();

    _float fProgress = m_pOwner->Get_AnimProgress();
    _float fMotionCancelProgress;
    if (COORDINATE_2D == eCoord)
        fMotionCancelProgress = m_f2DMotionCancelProgress[m_iComboCount];
    else
        fMotionCancelProgress = m_f3DMotionCancelProgress[m_iComboCount];
    if (COORDINATE_2D == eCoord &&
        fProgress >= m_f2DForwardStartProgress && fProgress <= m_f2DForwardEndProgress)
    {

        if (m_pOwner->Is_PlatformerMode() && false == m_pOwner->Is_OnGround())
        {
            if (false == m_bRised )
            {
                m_pOwner->Add_Impuls({ 0.f,m_f2DRisingForce,0.f });
                m_bRised = true;
            }
        }
        else
            m_pOwner->Move(m_pOwner->Get_AttackTargetDirection() * m_f2DForwardSpeed, _fTimeDelta);
    }
    if (fProgress >= fMotionCancelProgress)
    {
        if (m_pOwner->Is_AttackTriggerActive())
        {
            m_pOwner->End_Attack();
        }
        if (m_bCombo)
        {
            m_iComboCount++;
            if (iMaxComboCount - 1 >= m_iComboCount)
            {
                if (tKeyResult.bInputStates[PLAYER_INPUT_MOVE])
                {
                    if (COORDINATE_2D == eCoord)
                    {
 
                        E_DIRECTION eNewDir = To_EDirection(tKeyResult.vMoveDir);
                        F_DIRECTION eFDir = To_FDirection(eNewDir);
                        m_pOwner->Set_2DDirection(To_EDirection(tKeyResult.vDir));
                    }

                }
                Switch_To_AttackAnimation(m_iComboCount);
                m_pOwner->Start_Attack((CPlayer::ATTACK_TYPE)(CPlayer::ATTACK_TYPE_NORMAL1 + m_iComboCount));
            }
            m_bCombo = false;
        }
        else
        {
            if (tKeyResult.bInputStates[PLAYER_INPUT_JUMP])
                m_pOwner->Set_State(CPlayer::JUMP_UP);
            else if (tKeyResult.bInputStates[PLAYER_INPUT_ROLL])
                m_pOwner->Set_State(CPlayer::ROLL);
            else if (tKeyResult.bInputStates[PLAYER_INPUT_THROWSWORD])
                m_pOwner->Set_State(CPlayer::THROWSWORD);

        }
    }

}

void CPlayerState_Attack::Enter()
{
    PLAYER_INPUT_RESULT tKeyResult = m_pOwner->Player_KeyInput();
    COORDINATE eCoord = m_pOwner->Get_CurCoord();
    _vector fAttackDirection = m_pOwner->Get_AttackTargetDirection();


    if (COORDINATE_3D == eCoord)
    {
        if (m_pOwner->Is_Dynamic())
            m_pOwner->LookDirectionXZ_Dynamic(fAttackDirection);
        else
            m_pOwner->LookDirectionXZ_Kinematic(fAttackDirection);
        m_pOwner->Stop_Rotate();
    }
    else
    {
        m_f2DForwardSpeed = m_pOwner->Get_2DAttackForwardingSpeed();
        m_pOwner->Set_2DDirection(To_FDirection(fAttackDirection));
    }

    Switch_To_AttackAnimation(m_iComboCount);
    m_pOwner->Start_Attack(CPlayer::ATTACK_TYPE_NORMAL1);
}

void CPlayerState_Attack::Exit()
{
    m_pOwner->End_Attack();
}

void CPlayerState_Attack::On_AnimEnd(COORDINATE _eCoord, _uint _iAnimIdx)
{
    m_iComboCount = 0;
    m_bCombo = false;
    m_bRised = false;
    m_pOwner->Set_State(CPlayer::IDLE);
}

void CPlayerState_Attack::Switch_To_AttackAnimation(_uint _iComboCount)
{
    COORDINATE eCoord = m_pOwner->Get_CurCoord();

    if (COORDINATE_2D == eCoord)
    {
        F_DIRECTION eFDIr = To_FDirection(m_pOwner->Get_2DDirection());
        switch (eFDIr)
        {
        case Client::F_DIRECTION::LEFT:
        case Client::F_DIRECTION::RIGHT:
            switch (m_iComboCount)
            {
            case 0:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACK_RIGHT);
                break;
            case 1:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACKCOMBO_01_RIGHT);
                break;
            case 2:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACKCOMBO_03_RIGHT);
                break;
            default:
                break;
            }
            break;
        case Client::F_DIRECTION::UP:
            switch (m_iComboCount)
            {
            case 0:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACK_UP);
                break;
            case 1:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACKCOMBO_01_UP);
                break;
            case 2:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACKCOMBO_03_UP);
                break;
            default:
                break;
            }
            break;
        case Client::F_DIRECTION::DOWN:
            switch (m_iComboCount)
            {
            case 0:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACK_DOWN);
                break;
            case 1:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACKCOMBO_01_DOWN);
                break;
            case 2:
                m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_2D::PLAYER_ATTACKCOMBO_03_DOWN);
                break;
            default:
                break;
            }
            break;
        case Client::F_DIRECTION::F_DIR_LAST:
        default:
            break;
        }
    }
    else
    {
        switch (m_iComboCount)
        {
        case 0:
            m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_ATTACK_01_GT_EDIT);
            break;
        case 1:
            m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_ATTACK_02_GT);
            break;
        case 2:
            m_pOwner->Switch_Animation((_uint)CPlayer::ANIM_STATE_3D::LATCH_ANIM_ATTACK_03_GT);
            break;
        default:
            break;
        }
    }

    if (0 == m_iComboCount)
    {
        m_pGameInstance->Start_SFX_Delay(_wstring(L"A_sfx_small_efforts-") + to_wstring(rand() % m_iSmallEffortsSfxCount), m_fAttackSfxDelay, m_fVocalSfxVolume);
        m_pGameInstance->Start_SFX_Delay(_wstring(L"A_sfx_sword_swing-") + to_wstring(rand() % m_iSwordSwingSfxCount), m_fAttackSfxDelay, m_fSwordSfxVolume);
    }
    else if (1 == m_iComboCount)
    {
        m_pGameInstance->Start_SFX_Delay(_wstring(L"A_sfx_small_efforts-") + to_wstring(rand() % m_iSmallEffortsSfxCount), m_fAttackSfxDelay, m_fVocalSfxVolume);
        m_pGameInstance->Start_SFX_Delay(_wstring(L"A_sfx_sword_strike2-") + to_wstring(rand() % m_iSwordStrike2SfxCount), m_fAttackSfxDelay, m_fSwordSfxVolume);
    }
    else if (2 == m_iComboCount)
    {
        m_pGameInstance->Start_SFX_Delay(_wstring(L"A_sfx_jot_vocal_finalblow-") + to_wstring(rand() % m_iFinalBlowSfxCount), m_fAttackSfxDelay, m_fVocalSfxVolume);
        m_pGameInstance->Start_SFX_Delay(_wstring(L"A_sfx_sword_strike3-") + to_wstring(rand() % m_iSwordStrike3SfxCount), m_fAttackSfxDelay, m_fSwordSfxVolume);

    }

}

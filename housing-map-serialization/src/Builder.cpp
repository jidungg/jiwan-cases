#include "stdafx.h"
#include "Builder.h"

#include "GameInstance.h"
#include "ModelObject.h"
#include "Client_Utility.h"
#include "Terrain.h"
#include "TerrainObject.h"
#include "ItemDataBase.h"
#include "BuildPreview.h"
#include "EffModelObject.h"

HRESULT CBuilder::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	BUILDER_DESC* pBulderDesc = static_cast<BUILDER_DESC*>(pArg);
	m_pCubeTerrain = pBulderDesc->pCubeTerrain;

	if (FAILED(Ready_Builder()))
		return E_FAIL;

	if (FAILED(Ready_Preview(static_cast<BUILD_ITEM_DATA*>(ITEMDB->Get_Data(ITEM_TYPE::BUILD, m_iInitialPreviewItemID)))))
		return E_FAIL;

	if (FAILED(Ready_Marker()))
		return E_FAIL;
	return S_OK;
}
HRESULT CBuilder::Ready_Builder()
{
	//DuckyBall
	CModelObject::MODELOBJ_DESC tModelDesc;
	tModelDesc.fRotationPerSec = m_fObjectRotationPerSec;
	tModelDesc.fSpeedPerSec = m_fObjectSpeedPerSec;
	tModelDesc.eModelProtoLevelID = LEVEL_HOME;
	lstrcpyW(tModelDesc.szModelProtoName, m_szBirdModelProtoName);
	m_pBird = static_cast<CModelObject*>(m_pGameInstance->Clone_Proto_Object_Stock(CModelObject::m_szProtoTag, &tModelDesc));
	if (nullptr == m_pBird)
		return E_FAIL;
	Add_Child(m_pBird);
	m_pBird->Set_AnimationLoop(0, true);
	m_pBird->Get_Transform()->Set_State(CTransform::STATE_POSITION, m_vBirdOffset);

	return S_OK;
}
HRESULT CBuilder::Ready_Preview(BUILD_ITEM_DATA* pDesc)
{
	if (nullptr == pDesc)
		return E_FAIL;
	CTerrainObject::TERRAINOBJ_DESC tTerrObjDesc;
	tTerrObjDesc.fRotationPerSec = m_fObjectRotationPerSec;
	tTerrObjDesc.fSpeedPerSec = m_fObjectSpeedPerSec;
	tTerrObjDesc.iID =pDesc->iItemID;
	tTerrObjDesc.eModelProtoLevelID = LEVEL_LOADING;
	lstrcpyW(tTerrObjDesc.szModelProtoName, pDesc->strModelTag);
	XMStoreFloat4(&tTerrObjDesc.pos, m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_vPreviewOffset);

	m_pPreview = static_cast<CBuildPreview*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_HOME, CBuildPreview::m_szProtoTag, &tTerrObjDesc));
	if (nullptr == m_pPreview)
		return E_FAIL;
	Add_Child(m_pPreview);
	m_pPreview->Get_Transform()->Set_State(CTransform::STATE_POSITION, m_vPreviewOffset);
	m_pPreview->Get_Transform()->Scaling(m_vPreviewScale.x, m_vPreviewScale.y, m_vPreviewScale.z);
	return S_OK;
}

HRESULT CBuilder::Ready_Marker()
{
	CEffModelObject::EFFECTOBJ_DESC tEffModelDesc;
	strcpy_s(tEffModelDesc.strModelProtoName, "eff_add_cube_mark_enable.effmodel");
	tEffModelDesc.eModelProtoLevelID = LEVEL_LOADING;
	m_pCubeMarkerEnable = static_cast<CEffModelObject*>(m_pGameInstance->Clone_Proto_Object_Stock(CEffModelObject::m_szProtoTag, &tEffModelDesc));
	if (nullptr == m_pCubeMarkerEnable)
		return E_FAIL;

	strcpy_s(tEffModelDesc.strModelProtoName, "eff_add_cube_mark_disable.effmodel");
	tEffModelDesc.eModelProtoLevelID = LEVEL_LOADING;
	m_pCubeMarkerDisable = static_cast<CEffModelObject*>(m_pGameInstance->Clone_Proto_Object_Stock(CEffModelObject::m_szProtoTag, &tEffModelDesc));
	if (nullptr == m_pCubeMarkerDisable)
		return E_FAIL;


	m_pCubeMarkerEnable->Start_Animation(0, true);
	m_pCubeMarkerDisable->Start_Animation(0, true);

	return S_OK;
}

void CBuilder::Receive_KeyInput(_float fTimeDelta)
{
	if (m_pGameInstance->GetKeyState(KEY::RIGHT) == KEY_STATE::PRESSING)
		m_vMoveDir += Get_Direction_Vector(DIR_E);
	if (m_pGameInstance->GetKeyState(KEY::UP) == KEY_STATE::PRESSING)
		m_vMoveDir += Get_Direction_Vector(DIR_N);
	if (m_pGameInstance->GetKeyState(KEY::DOWN) == KEY_STATE::PRESSING)
		m_vMoveDir += Get_Direction_Vector(DIR_S);
	if (m_pGameInstance->GetKeyState(KEY::LEFT) == KEY_STATE::PRESSING)
		m_vMoveDir += Get_Direction_Vector(DIR_W);
	if (m_pGameInstance->GetKeyState(KEY::W) == KEY_STATE::PRESSING)
		m_vMoveDir += Get_Direction_Vector(DIR_U);
	if (m_pGameInstance->GetKeyState(KEY::S) == KEY_STATE::PRESSING)
		m_vMoveDir += Get_Direction_Vector(DIR_D);

#ifdef _DEBUG
	for (_uint key = 0; key < 10; key++)
	{
		if (m_pGameInstance->GetKeyState((KEY)((_uint)KEY::NUM0 + key)) == KEY_STATE::DOWN)
		{
			m_iBuildData = key;
			break;
		}
	}
	if (m_pGameInstance->GetKeyState(KEY::F1) == KEY_STATE::PRESSING)
		m_fBuildData -= m_fBuildDataDelta;
	if (m_pGameInstance->GetKeyState(KEY::F2) == KEY_STATE::PRESSING)
		m_fBuildData += m_fBuildDataDelta;
#endif

	_vector vPos;
	vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_uint iIndex = m_pCubeTerrain->PosToIndex(vPos);
	m_bBuildable = m_pCubeTerrain->Is_Buildable(vPos);
	if (m_pGameInstance->GetKeyState(KEY::R) == KEY_STATE::DOWN)
	{
		CTerrainObject* pTerrObj = m_pCubeTerrain->Get_TerrainObject(iIndex);
		if (pTerrObj)
			pTerrObj->Rotate();
		else
			m_pPreview->Rotate();
	}
	if (m_pGameInstance->GetKeyState(KEY::E) == KEY_STATE::DOWN) 
	{

		m_pCubeTerrain->Remove_TerrainObject(iIndex);

	}
	if (m_bBuildable && m_pGameInstance->GetKeyState(KEY::SPACE) == KEY_STATE::PRESSING)
	{
		BUILD_ITEM_DATA* itemdesc =static_cast<BUILD_ITEM_DATA*>(ITEMDB->Get_Data(ITEM_TYPE::BUILD, (_uint)m_pPreview->Get_BuildItemID()));
		if (nullptr == itemdesc)
			return;

		CTerrainObject::TERRAINOBJ_DESC desc;
		desc.fRotationPerSec = m_fObjectRotationPerSec;
		desc.fSpeedPerSec = m_fObjectSpeedPerSec;
		desc.iID = m_pPreview->Get_BuildItemID();
		desc.eModelProtoLevelID = LEVEL_LOADING;
		lstrcpyW(desc.szModelProtoName, itemdesc->strModelTag);
		desc.direction = m_pPreview->Get_Direction();
		desc.vecIData.push_back(m_iBuildData);
		desc.vecFData.push_back(m_fBuildData);
		desc.index = iIndex;
		desc.iParentIndex = iIndex;
		XMStoreFloat4(&desc.pos, m_pCubeTerrain->IndexToPos(desc.index));

		m_pCubeTerrain->Add_TerrainObject(desc);

	
	}
}

void CBuilder::Update(_float fTimeDelta)
{
	XMVECTOR vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	if (false == XMVector4Equal(m_vMoveDir, XMVectorSet(0, 0, 0, 0)))
	{
		m_pTransformCom->Go_Direction(m_vMoveDir, fTimeDelta);

		m_pTransformCom->LookToward(XMVectorSetY(m_vMoveDir, 0));
	}

	_vector vSnapPos = m_pCubeTerrain->SnapPosition(vPos);
	if (m_bBuildable)
	{
		m_pCubeMarkerDisable->Set_Active(false);
		m_pCubeMarkerEnable->Set_Active(true);
		m_pCubeMarkerEnable->Get_Transform()->Set_State(CTransform::STATE_POSITION, vSnapPos);
		m_pCubeMarkerEnable->Update(fTimeDelta);
	}
	else
	{
		m_pCubeMarkerEnable->Set_Active(false);
		m_pCubeMarkerEnable->Set_Active(true);
		m_pCubeMarkerDisable->Get_Transform()->Set_State(CTransform::STATE_POSITION, vSnapPos);
		m_pCubeMarkerDisable->Update(fTimeDelta);
	}

	__super::Update(fTimeDelta);
}

void CBuilder::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);

	m_vMoveDir = XMVectorSet(0, 0, 0, 0);

	if (m_bBuildable)
		m_pCubeMarkerEnable->Late_Update(fTimeDelta);
	else
		m_pCubeMarkerDisable->Late_Update(fTimeDelta);

}
HRESULT CBuilder::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	if (m_bBuildable)
		m_pCubeMarkerEnable->Render();
	else
		m_pCubeMarkerDisable->Render();
	return S_OK;
}


void CBuilder::Set_BuildItem(_uint eID)
{
	BUILD_ITEM_DATA* tItemDesc = static_cast<BUILD_ITEM_DATA*>(ITEMDB->Get_Data(ITEM_TYPE::BUILD, (_uint)eID));
	if (nullptr == tItemDesc)
		return;

	Remove_Child(m_pPreview);
	Safe_Release(m_pPreview);
	Ready_Preview(tItemDesc);
}

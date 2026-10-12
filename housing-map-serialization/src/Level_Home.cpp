#include "stdafx.h"
#include "Level_Home.h"

#include "GameInstance.h"
#include "Player.h"
#include "Camera_Trace.h"
#include "UIHomeDialog.h"
#include "Builder.h"
#include "Terrain.h"
#include "Collider.h"
#include "TerrainObject.h"
#include "ItemDataBase.h"

namespace
{
	constexpr _float fBuilderRotationPerSec = 10.f;
	constexpr _float fBuilderSpeedPerSec = 4.f;
	constexpr char szMyHomeJsonPath[] = "../Bin/Resources/Json/MyHome.json";
	const _float4 vHomeDialogBorder = { 3, 3, 3, 3 };
}

HRESULT CLevel_Home::Initialize(void* pArg)
{
	__super::Initialize(pArg);
	m_pItemData = ITEMDB->GetItemMap(ITEM_TYPE::BUILD);
	m_pItemIter = m_pItemData->begin();

	//CAMERA
	m_pCamera = static_cast<CCamera_Trace*>(m_pGameInstance->Get_FirstGameObject(LEVEL_HOME, LAYER_CAMERA));;

	//BUILDER
	CBuilder::BUILDER_DESC BuilderDesc{};
	BuilderDesc.fRotationPerSec = fBuilderRotationPerSec;
	BuilderDesc.fSpeedPerSec = fBuilderSpeedPerSec;
	BuilderDesc.pCubeTerrain = m_pCubeTerrain;
	m_pBuilder = static_cast<CBuilder*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_HOME, CBuilder::m_szProtoTag, &BuilderDesc));
	if (nullptr == m_pBuilder)
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_HOME, LAYER_NONCOLLISION, m_pBuilder)))
		return E_FAIL;
	m_pBuilder->Set_Active(false);

	Set_BuildItem(static_cast<BUILD_ITEM_DATA*>(m_pItemIter->second)->iItemID);

	Ready_Layer_UI(LAYER_UI);

	return S_OK;
}

void CLevel_Home::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	//----------------------------------------------
	if (m_pGameInstance->GetKeyState(KEY::B) == KEY_STATE::DOWN)
	{
		m_bBuildMode = !m_bBuildMode;
		m_pBuilder->Set_Active(m_bBuildMode);
		m_pHomeDialog->Set_Active(m_bBuildMode);
		if(m_bBuildMode)
		{
			m_pCamera->Set_Target(m_pBuilder);
			m_pGameInstance->Possess(m_pBuilder);
			XMVECTOR vPos = m_pPlayer->Get_Transform()->Get_State(CTransform::STATE_POSITION);
			_float4 fPos;
			XMStoreFloat4(&fPos, vPos);
			m_pBuilder->Set_Position(m_pCubeTerrain->IndexToPos(m_pCubeTerrain->PosToIndex(fPos)));
		}
		else
		{
			m_pCamera->Set_Target(m_pPlayer);
			m_pGameInstance->Possess(m_pPlayer);
		}
	}
	if (m_bBuildMode)
	{
		//터레인 피킹해서 해당 위치로 이동
		if (m_pGameInstance->GetMouseKeyState(MOUSE_KEY::LB) == KEY_STATE::UP)
		{
			POINT tPosition = m_pGameInstance->Get_MousePos();
			Ray tRay{ tPosition, g_iWinSizeX, g_iWinSizeY,
				m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW),
				m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ) };

			RAYCASTHIT tHitInfo;
			bool bHit = m_pGameInstance->RayCast(tRay, &tHitInfo);
			if (bHit)
			{
				CTerrainObject* pObj = dynamic_cast<CTerrainObject*>(tHitInfo.pCollider->Get_Owner());
				if (pObj == nullptr)
					return;
				_uint iIndex = pObj->Get_Index();
				XMUINT3 iSize = m_pCubeTerrain->Get_Size();
				iIndex += iSize.x * iSize.z;

				m_pBuilder->Set_Position( m_pCubeTerrain->IndexToPos(iIndex));
			}

		}
		//다음 아이템 선택
		if (m_pGameInstance->GetKeyState(KEY::F) == KEY_STATE::DOWN)
		{
			m_pItemIter++;
			if (m_pItemIter == m_pItemData->end())
				m_pItemIter = m_pItemData->begin();

			Set_BuildItem(m_pItemIter->first);
			m_pHomeDialog->Select_NextItem();
		}
		//맵 저장
		if (m_pGameInstance->GetKeyState(KEY::H) == KEY_STATE::DOWN)
		{
			m_pCubeTerrain->Save_To_Json(szMyHomeJsonPath);
		}
	}
}

void CLevel_Home::On_UIBuildItemSelected(void* pArg)
{
	CUIButtonItemIndicator* pIndicator = reinterpret_cast<CUIButtonItemIndicator*>(pArg);
	const BUILD_ITEM_DATA* pDesc = reinterpret_cast<const BUILD_ITEM_DATA*>(pIndicator->Get_ItemDesc());
	Set_BuildItem((pDesc)->iItemID);
}

void CLevel_Home::Set_BuildItem(_uint iID)
{
	m_pBuilder->Set_BuildItem(iID);
	m_pItemIter = m_pItemData->find(iID);
}


HRESULT CLevel_Home::Ready_Layer_UI(LAYERID eLayerId)
{
	CUIHomeDialog::HOMEDIALOG_DESC homDialogDesc{};
	homDialogDesc.eAnchorType = CORNOR_TYPE::RIGHT_BOT;
	homDialogDesc.ePivotType = CORNOR_TYPE::RIGHT_BOT;
	homDialogDesc.fXOffset = 0;
	homDialogDesc.fYOffset = 0;
	homDialogDesc.vBorder = vHomeDialogBorder;
	homDialogDesc.pTextureCom = static_cast<CTexture*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_COMPONENT, m_iLevelID, TEXT("UI_Texture_HomeDialog"), nullptr));
	list<const ITEM_DATA*> listData;
	for (auto& i : *m_pItemData)
		listData.push_back(i.second);
	homDialogDesc.listData = &listData;

	m_pHomeDialog = static_cast<CUIHomeDialog*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_HOME, TEXT("Prototype_GameObject_HomeDialog"), &homDialogDesc));
	if (nullptr == m_pHomeDialog)
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVELID::LEVEL_HOME, eLayerId, m_pHomeDialog)))
		return E_FAIL;
	function<void(void*)> f = bind(&CLevel_Home::On_UIBuildItemSelected, this, placeholders::_1);
	m_pHomeDialog->Register_OnClickCallback(f);
	m_pHomeDialog->Set_Active(false);
	return S_OK;
}

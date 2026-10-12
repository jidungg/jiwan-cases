#include "stdafx.h"
#include "UIHomeDialog.h"
#include "GameInstance.h"
#include "UIButton.h"
#include "UIScroller.h"

namespace
{
	const _float4 vButtonBorder = { 2, 2, 2, 2 };
	const _float4 vItemBackPanelBorder = { 4, 4, 4, 4 };
	constexpr _uint iItemEntryPoolSize = 200;
}

HRESULT CUIHomeDialog::Initialize(void* pArg)
{
	HOMEDIALOG_DESC* pDesc = static_cast<HOMEDIALOG_DESC*>(pArg);
	pDesc->fSizeX = m_fCommonMargin * (2 + m_iVisibleColCount + 1 + 1) + m_fItemWidth * m_iVisibleColCount + m_fCommonButtonSize;
	pDesc->fSizeY = m_fHeaderHeight + m_fCommonMargin * (1 + m_iVisibleRowCount + 1) + m_fItemHeight * m_iVisibleRowCount;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Childs( pDesc)))
		return E_FAIL;
	return S_OK;
}

void CUIHomeDialog::Select_NextItem()
{
	_uint iRow = m_pItemList->Get_ItemRow(m_pItemList->Select_NextItem());
	if (false == m_pItemList->Is_VisibleRow(iRow))
		m_pScroller->Set_CursorRow(iRow);
}

HRESULT CUIHomeDialog::Ready_Childs(HOMEDIALOG_DESC* pDesc)
{
	CUIButton::BUTTON_DESC ButtonDesc{};
	ButtonDesc.eAnchorType = CORNOR_TYPE::RIGHT_TOP;
	ButtonDesc.ePivotType = CORNOR_TYPE::RIGHT_TOP;
	ButtonDesc.fXOffset = -m_fCommonMargin;
	ButtonDesc.fYOffset = (m_fHeaderHeight - m_fCommonButtonSize) / 2;
	ButtonDesc.fSizeX = m_fCommonButtonSize;
	ButtonDesc.fSizeY = m_fCommonButtonSize;
	ButtonDesc.vBorder = vButtonBorder;
	ButtonDesc.pTextureCom = static_cast<CTexture*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_COMPONENT, LEVEL_HOME, TEXT("UI_Texture_Magnifier"), nullptr));
	if (nullptr == ButtonDesc.pTextureCom)
		return E_FAIL;

	CUIObject* pButton = CUIButton::Create(m_pDevice, m_pContext);
	if (nullptr == pButton)
		return E_FAIL;
	if (FAILED(pButton->Initialize(&ButtonDesc)))
		return E_FAIL;
	Add_Child(pButton);

	CUIPanel::PANEL_DESC tItemBackPanelDesc;
	_float3 fSize = m_pTransformCom->Compute_Scaled();
	tItemBackPanelDesc.eAnchorType = CORNOR_TYPE::LEFT_TOP;
	tItemBackPanelDesc.ePivotType = CORNOR_TYPE::LEFT_TOP;
	tItemBackPanelDesc.fXOffset = m_fCommonMargin;
	tItemBackPanelDesc.fYOffset = m_fHeaderHeight;
	tItemBackPanelDesc.fSizeX = fSize.x - m_fCommonMargin*2;
	tItemBackPanelDesc.fSizeY = fSize.y - m_fHeaderHeight - m_fCommonMargin ;
	tItemBackPanelDesc.pTextureCom = static_cast<CTexture*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_COMPONENT, LEVEL_LOADING, TEXT("UI_Texture_ItemListBack"), nullptr));
	tItemBackPanelDesc.vBorder = vItemBackPanelBorder;
	m_pItemBackPanel = static_cast<CUIPanel*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_LOADING, CUIPanel::m_szProtoTag, &tItemBackPanelDesc));
	if (nullptr == m_pItemBackPanel)
		return E_FAIL;
	Add_Child(m_pItemBackPanel);

	CUIListSelector::UILISTSELECTOR_DESC ListDesc{};
	ListDesc.eAnchorType = CORNOR_TYPE::LEFT_TOP;
	ListDesc.ePivotType = CORNOR_TYPE::LEFT_TOP;
	ListDesc.fXOffset = m_fCommonMargin;
	ListDesc.fYOffset = m_fCommonMargin;
	ListDesc.fSizeX = tItemBackPanelDesc.fSizeX - m_fCommonButtonSize - m_fCommonMargin*3;
	ListDesc.fSizeY = tItemBackPanelDesc.fSizeY - m_fCommonMargin *2;
	ListDesc.fItemHeight = m_fItemHeight;
	ListDesc.fItemWidth = m_fItemHeight;
	ListDesc.fItemMarginX = m_fCommonMargin;
	ListDesc.fItemMarginY = m_fCommonMargin;
	ListDesc.iColumnCount = m_iVisibleColCount;
	ListDesc.iRowCount = (_uint)ceilf((_float)pDesc->listData->size() / (_float)m_iVisibleColCount);;
	ListDesc.eHighlighterTexProtoLev = LEVEL_LOADING;
	ListDesc.szHighlighterTexProtoTag = TEXT("UI_Texture_HighlightBorder");
	ListDesc.eItemEntryProtoLev = LEVEL_HOME;
	ListDesc.szItemEntryProtoTag = TEXT("Prototype_GameObject_HomeDialogBuildItemIndicator");
	ListDesc.iObjectPoolSize = iItemEntryPoolSize;
	m_pItemList = static_cast<CUIListSelector*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_LOADING, TEXT("Prototype_GameObject_UIList"), &ListDesc));
	if (nullptr == m_pItemList)
		return E_FAIL;
	m_pItemBackPanel->Add_Child(m_pItemList);
	m_pItemList->Set_ItemData(pDesc->listData);

	CUIScroller::SCROLLBAR_DESC ScrollDesc{};
	ScrollDesc.eAnchorType = CORNOR_TYPE::RIGHT_BOT;
	ScrollDesc.ePivotType = CORNOR_TYPE::RIGHT_BOT;
	ScrollDesc.fXOffset = -m_fCommonMargin;
	ScrollDesc.fYOffset = -m_fCommonMargin;
	ScrollDesc.fSizeX = m_fCommonButtonSize;
	ScrollDesc.fSizeY = ListDesc.fSizeY;
	ScrollDesc.iTotalRowCount = m_pItemList->Get_RowCount();
	ScrollDesc.iVisibleRowCount = m_iVisibleRowCount;
	ScrollDesc.pUIList = m_pItemList;
	m_pScroller = static_cast<CUIScroller*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_LOADING, CUIScroller::m_szProtoTag, &ScrollDesc));
	if (nullptr == m_pScroller)
		return E_FAIL;
	m_pItemBackPanel->Add_Child(m_pScroller);

	return S_OK;
}

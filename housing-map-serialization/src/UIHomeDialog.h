#pragma once
#include "Client_Defines.h"
#include "UIListSelector.h"
#include "UIItemIndicator.h"

BEGIN(Client)
class CUIScroller;
class CUIHomeDialog :
	public CUIPanel
{
public:
	typedef struct PanelDesc: public CUIPanel::PANEL_DESC
	{
		list<const ITEM_DATA*>* listData;
	}HOMEDIALOG_DESC;

public:
	virtual HRESULT Initialize(void* pArg) override;

	void Register_OnClickCallback(const ButtonCallback& fCallback){m_pItemList->Register_OnClickCallback(fCallback);}
	void Select_NextItem();
private:
	HRESULT Ready_Childs(HOMEDIALOG_DESC* pDesc);
	CUIPanel* m_pItemBackPanel = { nullptr };
	CUIListSelector* m_pItemList = { nullptr };
	CUIScroller* m_pScroller = { nullptr };

	_float m_fCommonMargin = 5;
	_float m_fCommonButtonSize = 24;
	_float m_fHeaderHeight = 40;
	_float m_fItemHeight = 50;
	_float m_fItemWidth = 50;
	_uint m_iVisibleRowCount = 2;
	_uint m_iVisibleColCount = 10;

	_float4 m_vButtonBorder = { 2, 2, 2, 2 };
	_float4 m_vItemBackPanelBorder = { 4, 4, 4, 4 };
	_uint m_iItemEntryPoolSize = 200;
};

END
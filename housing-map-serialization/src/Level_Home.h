#pragma once
#include "Client_Defines.h"
#include "Level_GamePlay.h"
#include "ItemDataBase.h"


BEGIN(Client)
class CBuilder;
class CCamera_Trace;
class CUIHomeDialog;
class CLevel_Home final:
	public CLevel_GamePlay
{
public:
	virtual HRESULT Initialize(void* pArg)override;
	virtual void Update(_float fTimeDelta)override;

	void On_UIBuildItemSelected(void*);
	void Set_BuildItem(_uint iID);
private:
	HRESULT Ready_Layer_UI(LAYERID eLayerId);

private:

	CBuilder* m_pBuilder = { nullptr };
	CCamera_Trace* m_pCamera = { nullptr };

	map<_uint,  ITEM_DATA*>* m_pItemData = nullptr;
	map<_uint,  ITEM_DATA*>::iterator m_pItemIter = {};
	bool m_bBuildMode = false;

	CUIHomeDialog* m_pHomeDialog = { nullptr };
};

END

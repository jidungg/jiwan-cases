#pragma once

#include "Base.h"
#include "Item.h"
BEGIN(Engine)
class CRenderTarget;
END
BEGIN(Client)
class CItemDataBase :
	public CBase
{
	DECLARE_SINGLETON(CItemDataBase)

public:
	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	HRESULT Load_Data();
	void Insert_Data(ITEM_DATA* pData );
	ITEM_DATA* Get_Data(ITEM_TYPE eType, _uint iKey)
	{
		auto iter = m_mapItem[(_uint)eType].find(iKey);
		if (iter == m_mapItem[(_uint)eType].end())
			return nullptr;
		return iter->second;
	}
	map<_uint, ITEM_DATA*>* GetItemMap(ITEM_TYPE eType) { return &m_mapItem[(_uint)eType]; }
private:
	map<_uint, ITEM_DATA*> m_mapItem[(_uint)ITEM_TYPE::LAST];


	// 빌드 아이템 아이콘 제작용
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CRenderTarget* m_pRenderTarget = { nullptr };
	ID3D11RenderTargetView* m_pBackRTV = { nullptr };
	ID3D11DepthStencilView* m_pOriginalDSV = { nullptr };
	_float4 m_vClearColor = {};
	CShader* m_pShader = { nullptr };
	_float4x4 m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
};
#define ITEMDB CItemDataBase::GetInstance()
END
#pragma once
#include "Client_Defines.h"
#include "Pawn.h"
#include "Item.h"


BEGIN(Client)
class CModelObject;
class CEffModelObject;
class CTerrain;
class CBuildPreview;
class CBuilder :
	public CPawn
{
public:
	typedef struct tagBuilderDesc : public CPawn::GAMEOBJECT_DESC
	{
		CTerrain* pCubeTerrain = nullptr;

	}BUILDER_DESC;

	static constexpr _tchar m_szProtoTag[] = L"Prototype_GameObject_Builder";

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Receive_KeyInput(_float fTimeDelta)  override;
	virtual void Late_Update(_float fTimeDelta) override;

	void Set_BuildItem(_uint eID);

private:
	HRESULT Ready_Builder();
	HRESULT Ready_Preview(BUILD_ITEM_DATA* pDesc);
	HRESULT Ready_Marker();
private:
	CTerrain* m_pCubeTerrain = { nullptr };

	_float m_fObjectRotationPerSec = 5.f;
	_float m_fObjectSpeedPerSec = 1.f;

	CModelObject* m_pBird = { nullptr };
	XMVECTOR m_vBirdOffset = XMVectorSet(0, 1, 0, 1);
	static constexpr _tchar m_szBirdModelProtoName[] = L"60100001_chocoduckyball.model";

	CBuildPreview* m_pPreview = { nullptr };
	XMVECTOR m_vPreviewOffset = XMVectorSet(0, 0.5f, 0, 1);
	_float3 m_vPreviewScale = { 0.5f, 0.5f, 0.5f };
	_uint m_iInitialPreviewItemID = 0;
	_int m_iBuildData = 0;
	_float m_fBuildData = 5.f;
	_float m_fBuildDataDelta = 0.1f;
	XMVECTOR m_vMoveDir = XMVectorSet(0, 0, 0, 0);

	_bool m_bBuildable = { true };
	CEffModelObject* m_pCubeMarkerEnable = { nullptr };
	CEffModelObject* m_pCubeMarkerDisable = { nullptr };
};

END
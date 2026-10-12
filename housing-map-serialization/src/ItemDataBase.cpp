#include "stdafx.h"
#include "ItemDataBase.h"
#include "JsonParser.h"
#include "GameInstance.h"
#include "RenderTarget.h"
#include "Model.h"

IMPLEMENT_SINGLETON(CItemDataBase)
constexpr int DEFAULT_PASS = 1;
constexpr _uint ICON_SIZE = 300;

HRESULT CItemDataBase::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	{
		m_pDevice = pDevice;
		m_pContext = pContext;
		Safe_AddRef(m_pDevice);
		Safe_AddRef(m_pContext);
	}

	m_vClearColor = _float4(0.f, 0.f, 0.f, 0.f);
	m_pRenderTarget = CRenderTarget::Create(m_pDevice, m_pContext, ICON_SIZE, ICON_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, m_vClearColor);
	if (nullptr == m_pRenderTarget)
		return E_FAIL;


	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	m_pShader = static_cast<CShader*>(pGameInstance->Clone_Proto_Component_Stock(TEXT("Prototype_Component_Shader_VtxMesh")));
	if (nullptr == m_pShader)
		return E_FAIL;
	_vector vAt{ 0,0.5,0,1 };
	_vector vPosition{ -1.5,0.5,-1.5,1 };
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixLookAtLH(vPosition, vAt, vUp));
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	_float fAspect = 1;
	_float fFovY = XMConvertToRadians(60.f);
	_float fNear = 0.1f;
	_float fFar = 1000.f;
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixPerspectiveFovLH(fFovY, fAspect, fNear, fFar));

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	_uint iNumViewports = 1;
	D3D11_VIEWPORT tOriginalViewport = {};
	pContext->RSGetViewports(&iNumViewports, &tOriginalViewport);
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (_float)ICON_SIZE;
	viewport.Height = (_float)ICON_SIZE;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &viewport);

	//Make Icon Img
	m_pContext->OMGetRenderTargets(1, &m_pBackRTV, &m_pOriginalDSV);
	ID3D11RenderTargetView* pRenderTargets[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	pRenderTargets[0] = m_pRenderTarget->Get_RTV();
	for (auto& pairBuildItemData : m_mapItem[(_uint)ITEM_TYPE::BUILD])
	{
		pGameInstance->Render_Begin();
		//Render Target 초기화
		{
			m_pRenderTarget->Clear();
			m_pContext->OMSetRenderTargets(1, pRenderTargets, m_pOriginalDSV);
		}

		//Render Target에 오브젝트 그리기
		CModel* pModel = static_cast<CModel*>(pGameInstance->Clone_Proto_Component_Stock(pairBuildItemData.second->strModelTag));
		if (nullptr == pModel)
			return E_FAIL;
		_uint		iNumMeshes = pModel->Get_NumMeshes();

		for (_uint i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(pModel->Bind_Material(m_pShader, "g_DiffuseTexture", i, TEXTURE_TYPE::DIFFUSE, 0)))
				return E_FAIL;
			if (FAILED(m_pShader->Begin(DEFAULT_PASS)))
				return E_FAIL;
			if (FAILED(pModel->Render(i)))
				return E_FAIL;
		}
		Safe_Release(pModel);

		//SRV 복사
		ID3D11Resource* pSourceResource = nullptr;
		ID3D11Resource* pDestResource = nullptr;
		m_pRenderTarget->Get_SRV()->GetResource(&pSourceResource);
		D3D11_TEXTURE2D_DESC desc = {};
		((ID3D11Texture2D*)pSourceResource)->GetDesc(&desc);
		ID3D11Texture2D* pDestTexture = nullptr;
		HRESULT hr = m_pDevice->CreateTexture2D(&desc, nullptr, &pDestTexture);
		if (FAILED(hr))
			return E_FAIL;
		pDestResource = pDestTexture;
		m_pContext->CopyResource(pDestResource, pSourceResource);
		ID3D11ShaderResourceView* pNewSRV = nullptr;
		m_pDevice->CreateShaderResourceView(pDestResource, nullptr, &pNewSRV);


		//Texture Prototype 만들기
		CTexture* pTexCom = CTexture::Create(m_pDevice, m_pContext, pNewSRV);
		if (nullptr == pTexCom)
			return E_FAIL;
		wstring wstrFIleName = pairBuildItemData.second->strModelTag;
		wstring wstrIconTag = wstrFIleName.substr(0, wstrFIleName.find_first_of(L"."));
		wstrIconTag += L"_icon";
		if(FAILED(pGameInstance->Add_Prototype(LEVEL_LOADING, wstrIconTag, pTexCom)))
			Safe_Release(pTexCom);

		{
			Safe_Release(pDestResource);
			Safe_Release(pSourceResource);
			m_pContext->OMSetRenderTargets(1, &m_pBackRTV, m_pOriginalDSV);
		}

		pGameInstance->Render_End();
	}
	Safe_Release(m_pOriginalDSV);
	Safe_Release(m_pBackRTV);

	pContext->RSSetViewports(1, &tOriginalViewport);

	Safe_Release(m_pRenderTarget);
	return S_OK;
}

HRESULT CItemDataBase::Load_Data()
{
	fs::path path;
	path = TEXT("../Bin/resources/FBXs/MAP");
	_uint iItemID = 0;
	for (const auto& entry : recursive_directory_iterator(path)) {
		if (entry.path().extension() == ".model") {
			string strFileName = entry.path().filename().string();
			BUILD_ITEM_DATA* pItemDesc = new BUILD_ITEM_DATA(strFileName, m_pDevice, m_pContext);
			pItemDesc->iItemID = iItemID++;
			Insert_Data(pItemDesc);

		}
	}

	json j;
	ITEM_DATA* pItemDesc;
	if (FAILED(CJsonParser::ReadJsonFile("../Bin/Resources/Json/ETCBuildItemData.json", &j)))
		return E_FAIL;
	for (auto& item : j["items"])
	{
		pItemDesc = new BUILD_ITEM_DATA(item);
		Insert_Data(pItemDesc);
	}

	j.clear();
	if (FAILED(CJsonParser::ReadJsonFile("../Bin/Resources/Json/EquipItemData.json", &j)))
		return E_FAIL;
	for (auto& item : j["items"])
	{
		pItemDesc = new EQUIP_ITEM_DATA(item);
		Insert_Data(pItemDesc);
	}

	j.clear();
	if (FAILED(CJsonParser::ReadJsonFile("../Bin/Resources/Json/ConsumableItemData.json", &j)))
		return E_FAIL;
	for (auto& item : j["items"])
	{
		pItemDesc = new CONSUMABLE_ITEM_DATA(item);
		Insert_Data(pItemDesc);
	}

	j.clear();
	if (FAILED(CJsonParser::ReadJsonFile("../Bin/Resources/Json/DecoItemData.json", &j)))
		return E_FAIL;
	for (auto& item : j["items"])
	{
		pItemDesc = new DECO_ITEM_DATA(item);
		Insert_Data(pItemDesc);
	}

	j.clear();
	if (FAILED(CJsonParser::ReadJsonFile("../Bin/Resources/Json/ETCItemData.json", &j)))
		return E_FAIL;
	for (auto& item : j["items"])
	{
		pItemDesc = new ETC_ITEM_DATA(item);
		Insert_Data(pItemDesc);
	}
	return S_OK;
}

void CItemDataBase::Insert_Data(ITEM_DATA* pData)
{
	m_mapItem[(_uint)pData->eITemType].insert({ pData->iItemID, pData });
}

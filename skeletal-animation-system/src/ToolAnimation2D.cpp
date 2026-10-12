#include "stdafx.h"
#include "ToolAnimation2D.h"


HRESULT CToolSpriteFrame::Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, json& _jData, const filesystem::path& _RootDir, map<string, CTexture*>& _Textures)
{
	json& jProperties = _jData["Properties"];

	json& jBakedSourceTexture = jProperties["BakedSourceTexture"];

	string strRootFolderName = _RootDir.parent_path().filename().string();
	filesystem::path pathObjectPath = jBakedSourceTexture["ObjectPath"].get<string>();
	string  strTextureKey = MakeTextureKey(pathObjectPath);
	if (_Textures.find(strTextureKey) == _Textures.end())
	{
		//ObjectPath에서 Root의 이름과 같은 폴더를 만날 때 까지 거슬러 올라감
		//Root의 이름과 같은 폴더를 만나면 그 폴더를 기준으로 상대경로를 만들어서 Texture를 로드함.
		filesystem::path pathCurFolderName = pathObjectPath.parent_path().filename();
		filesystem::path pathSubPath = pathObjectPath.filename().replace_extension(".dds");
		while (strRootFolderName != pathCurFolderName)
		{
			pathSubPath = pathCurFolderName / pathSubPath;
			string strParentPath = pathObjectPath.parent_path().string();
			if (strParentPath == "/")
			{
				pathSubPath = jBakedSourceTexture["ObjectPath"].get<string>();
				pathSubPath = pathSubPath.filename().replace_extension(".dds");
				break;
			}
			pathObjectPath = strParentPath;
			pathCurFolderName = pathObjectPath.parent_path().filename().string();
		}
		ID3D11ShaderResourceView* pSRV = { nullptr };
		//정확한 경로에 들어있으면 그대로 로드
		if (filesystem::exists(_RootDir / (pathSubPath)))
		{
			m_pathFinalTexturePath = _RootDir / (pathSubPath);
		}
		//정확한 경로에 없으면 이름이 같은 텍스처라도 찾아봄.
		else
		{
			string strSubFileName = pathSubPath.filename().string();
			for (const auto& entry : std::filesystem::recursive_directory_iterator(_RootDir))
			{
				if (".dds" == entry.path().extension()
					|| ".DDS" == entry.path().extension())
				{
					if (entry.path().filename().string() == strSubFileName)
					{
						m_pathFinalTexturePath = entry.path();
						break;
					}
				}
			}
		}
		if (m_pathFinalTexturePath.empty())
		{
			return E_FAIL;
		}
		HRESULT hr = DirectX::CreateWICTextureFromFile(_pDevice, m_pathFinalTexturePath.wstring().c_str(), nullptr, &pSRV);
		if (FAILED(hr))
		{
			m_pathFinalTexturePath.replace_extension(".dds");
			if (FAILED(DirectX::CreateWICTextureFromFile(_pDevice, m_pathFinalTexturePath.wstring().c_str(), nullptr, &pSRV)))
			{
				return E_FAIL;
			}
		}
		m_pTexture = CTexture::Create(_pDevice, _pContext);
		if (nullptr == m_pTexture)
			return E_FAIL;
		m_pTexture->Add_Texture(pSRV, StringToWstring( strTextureKey));

		_Textures.insert({strTextureKey, m_pTexture });
	}
	else
	{
		m_pTexture = _Textures[strTextureKey];
	}

	m_fPixelsPerUnrealUnit = jProperties["PixelsPerUnrealUnit"];

	json& jBakedRenderData = jProperties["BakedRenderData"];
	m_vSpriteStartUV = { 1.f,1.f };
	m_vSpriteEndUV = { 0.f,0.f };
	_float fMinX = D3D11_FLOAT32_MAX, fMinY = D3D11_FLOAT32_MAX;
	_float fMaxX = -D3D11_FLOAT32_MAX, fMaxY = -D3D11_FLOAT32_MAX;
	for (json& j : jBakedRenderData)
	{
		vBakedRenderDatas.push_back(_float4{ j["X"], j["Y"], j["Z"], j["W"] });
		m_vSpriteStartUV = _float2(min(vBakedRenderDatas.back().z, m_vSpriteStartUV.x), min(vBakedRenderDatas.back().w, m_vSpriteStartUV.y));
		m_vSpriteEndUV = _float2(max(vBakedRenderDatas.back().z, m_vSpriteEndUV.x), max(vBakedRenderDatas.back().w, m_vSpriteEndUV.y));
		fMinX = min(fMinX, vBakedRenderDatas.back().x);
		fMinY = min(fMinY, vBakedRenderDatas.back().y);
		fMaxX = max(fMaxX, vBakedRenderDatas.back().x);
		fMaxY = max(fMaxY, vBakedRenderDatas.back().y);
	}
	_float fWidth = abs(fMaxX - fMinX);
	_float fHeight = abs(fMaxY - fMinY);
	_float fXOffset = fMinX + fWidth / 2;
	_float fYOffset = fMinY + fHeight / 2;
	m_matSpriteTransform = XMMatrixScaling(fWidth, fHeight, 1);
	m_matSpriteTransform *= XMMatrixTranslation(fXOffset, fYOffset, 0);
	return S_OK;
}

HRESULT CToolSpriteFrame::Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures)
{

	//StartUV
	_inFile.read(reinterpret_cast<char*>(&m_vSpriteStartUV), sizeof(_float2));
	//EndUV
	_inFile.read(reinterpret_cast<char*>(&m_vSpriteEndUV), sizeof(_float2));
	//PixelsPerUnrealUnit
	_inFile.read(reinterpret_cast<char*>(&m_fPixelsPerUnrealUnit), sizeof(_float));
	//Texture
	_uint iCount = 0;
	_inFile.read(reinterpret_cast<char*>(&iCount), sizeof(_uint));
	if (iCount >= MAX_PATH)
		return E_FAIL;
	_char szTextureName[MAX_PATH] = "";
	_inFile.read(szTextureName, iCount);
	szTextureName[iCount] = '\0';
	m_pathFinalTexturePath = _szDirPath;
	m_pathFinalTexturePath /= szTextureName;
	m_pathFinalTexturePath += L".dds";
	auto pairTexture = _Textures.find(szTextureName);
	if (pairTexture == _Textures.end())
		return E_FAIL;
	m_pTexture = pairTexture->second;
	if (nullptr == m_pTexture)
		return E_FAIL;
	_inFile.read(reinterpret_cast<char*>(&m_matSpriteTransform), sizeof(_matrix));
	return S_OK;
}

HRESULT CToolSpriteFrame::Export(ofstream& _outfile)
{
	_outfile.write(reinterpret_cast<const char*>(&m_vSpriteStartUV), sizeof(_float2));
	_outfile.write(reinterpret_cast<const char*>(&m_vSpriteEndUV), sizeof(_float2));
	_outfile.write(reinterpret_cast<const char*>(&m_fPixelsPerUnrealUnit), sizeof(_float));
	_uint iCount = 0;
	string strTextureName = WstringToString(*m_pTexture->Get_SRVName(0));
	iCount = (_uint)strTextureName.length();
	_outfile.write(reinterpret_cast<const char*>(&iCount), sizeof(_uint));
	_outfile.write(strTextureName.c_str(), iCount);
	_outfile.write(reinterpret_cast<const char*>(&m_matSpriteTransform), sizeof(_matrix));
	return S_OK;
}

HRESULT CToolSpriteFrame::Copy_Textures(const filesystem::path& _szDestPath)
{
	if (false == filesystem::exists(m_pathFinalTexturePath))
	{
		return E_FAIL;
	}

	filesystem::path dest = _szDestPath;
	dest /= *m_pTexture->Get_SRVName(0) + L".dds";
	filesystem::copy_file(m_pathFinalTexturePath, dest, filesystem::copy_options::overwrite_existing);
	return S_OK;
}

CToolSpriteFrame* CToolSpriteFrame::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, json& _jData, const filesystem::path& _RootDir, map<string, CTexture*>& _Textures)
{
	CToolSpriteFrame* pInstance = new CToolSpriteFrame();

	if (FAILED(pInstance->Initialize(_pDevice, _pContext, _jData, _RootDir, _Textures)))
	{
		MSG_BOX("ToolSpriteFrame Create Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CToolSpriteFrame* CToolSpriteFrame::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures)
{
	CToolSpriteFrame* pInstance = new CToolSpriteFrame();

	if (FAILED(static_cast<CSpriteFrame*>(pInstance)->Initialize(_pDevice, _pContext, _szDirPath, _inFile, _Textures)))
	{
		MSG_BOX("ToolSpriteFrame Create Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

//===========================================ANIMATION2D===========================================


HRESULT CToolAnimation2D::Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, json& _jData, map<string, json>& _jPaperSprites, const filesystem::path& _RootDir, map<string, CTexture*>& _Textures)
{
	m_strName = _jData["Name"];
	json& jProperties = _jData["Properties"];
	if (jProperties.contains("FramesPerSecond"))
		m_fFramesPerSecond = jProperties["FramesPerSecond"];

	json& jKeyFrames = jProperties["KeyFrames"];
	for (json& jKeyFrame : jKeyFrames)
	{
		_uint iFrameRun = jKeyFrame["FrameRun"];

		if (jKeyFrame["Sprite"].is_null())
			continue;
		string strSpriteKey = filesystem::path(jKeyFrame["Sprite"]["ObjectPath"].get<string>()).filename().replace_extension().string();
		auto pairSpriteFile = _jPaperSprites.find(strSpriteKey);
		if (pairSpriteFile == _jPaperSprites.end())
			return E_FAIL;
		json& jSpriteFile = pairSpriteFile->second;

		CToolSpriteFrame* pSpriteFrame = CToolSpriteFrame::Create(_pDevice, _pContext, jSpriteFile, _RootDir, _Textures);
		if (nullptr == pSpriteFrame)
			return E_FAIL;
		m_SpriteFrames.push_back({ pSpriteFrame, iFrameRun });
	}
	m_iFrameCount = (_uint)m_SpriteFrames.size();
	return S_OK; 
}

HRESULT CToolAnimation2D::Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures)
{

	_uint iCount = 0;
	//Name
	_inFile.read(reinterpret_cast<char*>(&iCount), sizeof(_uint));
	m_strName.resize(iCount);
	_inFile.read(const_cast<char*>(m_strName.c_str()), iCount);
	//FramesPerSecond
	_inFile.read(reinterpret_cast<char*>(&m_fFramesPerSecond), sizeof(_float));
	//FrameCount
	_inFile.read(reinterpret_cast<char*>(&m_iFrameCount), sizeof(_uint));
	//Loop
	_inFile.read(reinterpret_cast<char*>(&m_bLoop), sizeof(_bool));
	//SpeedMagnifier
	_inFile.read(reinterpret_cast<char*>(&m_fSpeedMagnifier), sizeof(_float));
	//SpriteFrames
	_inFile.read(reinterpret_cast<char*>(&iCount), sizeof(_uint));
	m_SpriteFrames.reserve(iCount);
	for (_uint i = 0; i < iCount; i++)
	{
		CToolSpriteFrame* pSpriteFrame = CToolSpriteFrame::Create(_pDevice, _pContext, _szDirPath, _inFile, _Textures);
		if (nullptr == pSpriteFrame)
			return E_FAIL;
		_uint iFrameRun = 0;
		_inFile.read(reinterpret_cast<char*>(&iFrameRun), sizeof(_uint));
		m_SpriteFrames.push_back({ pSpriteFrame,iFrameRun });
	}

	return S_OK;
}
HRESULT CToolAnimation2D::Export(ofstream& _outfile)
{
	_uint iCount = 0;
	//Name
	iCount = (_uint)m_strName.length();
	_outfile.write(reinterpret_cast<const char*>(&iCount), sizeof(_uint));
	_outfile.write(m_strName.c_str(), iCount);
	//FramesPerSecond
	_outfile.write(reinterpret_cast<const char*>(&m_fFramesPerSecond), sizeof(_float));
	//FrameCount
	_outfile.write(reinterpret_cast<const char*>(&m_iFrameCount), sizeof(_uint));
	//Loop
	_outfile.write(reinterpret_cast<const char*>(&m_bLoop), sizeof(_bool));
	//SpeedMagnifier
	_outfile.write(reinterpret_cast<const char*>(&m_fSpeedMagnifier), sizeof(_float));
	
	//SpriteFrames
	iCount = (_uint)m_SpriteFrames.size();
	_outfile.write(reinterpret_cast<const char*>(&iCount), sizeof(_uint));
	for (auto& pairSpriteFrame : m_SpriteFrames)
	{
		static_cast<CToolSpriteFrame*>(pairSpriteFrame.first)->Export(_outfile);
		_outfile.write(reinterpret_cast<const char*>(&pairSpriteFrame.second), sizeof(_uint));
	}
	return S_OK;
}

HRESULT CToolAnimation2D::Copy_Textures(const filesystem::path& _szDestPath)
{
	for (auto& pairSpriteFrame : m_SpriteFrames)
	{
		static_cast<CToolSpriteFrame*>(pairSpriteFrame.first)->Copy_Textures(_szDestPath);
	}
	return S_OK;
}

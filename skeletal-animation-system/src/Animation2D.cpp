#include "Animation2D.h"
#include "GameInstance.h"

HRESULT CSpriteFrame::Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures)
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
	auto pairTexture = _Textures.find(szTextureName);
	if (pairTexture == _Textures.end())
		return E_FAIL;
	m_pTexture = pairTexture->second;
	Safe_AddRef(m_pTexture);
	if (nullptr == m_pTexture)
		return E_FAIL;
	_inFile.read(reinterpret_cast<char*>(&m_matSpriteTransform), sizeof(_matrix));
	return S_OK;
}

//39.8, 47.0
HRESULT CSpriteFrame::Bind_ShaderResource(CShader* _pShader)
{
	if (FAILED(m_pTexture->Bind_ShaderResource(_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(_pShader->Bind_RawValue("g_vSpriteStartUV", &m_vSpriteStartUV, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(_pShader->Bind_RawValue("g_vSpriteEndUV", &m_vSpriteEndUV, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(_pShader->Bind_RawValue("g_fPixelsPerUnrealUnit", &m_fPixelsPerUnrealUnit, sizeof(_float))))
		return E_FAIL;
	return S_OK;
}


CSpriteFrame* CSpriteFrame::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures)
{
	CSpriteFrame* pInstance = new CSpriteFrame();

	if (FAILED(pInstance->Initialize(_pDevice, _pContext, _szDirPath, _inFile, _Textures)))
	{
		MSG_BOX("SpriteFrame Create Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

//==============================Animation2D==============================

HRESULT CAnimation2D::Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures)
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
		CSpriteFrame* pSpriteFrame = CSpriteFrame::Create(_pDevice, _pContext, _szDirPath, _inFile, _Textures);
		if (nullptr == pSpriteFrame)
			return E_FAIL;
		_uint iFrameRun = 0;
		_inFile.read(reinterpret_cast<char*>(&iFrameRun), sizeof(_uint));
		m_SpriteFrames.push_back({ pSpriteFrame,iFrameRun });
	}

	return S_OK;
}

HRESULT CAnimation2D::Bind_ShaderResource(CShader* _pShader)
{
	if (FAILED(m_SpriteFrames[m_iCurrentFrame].first->Bind_ShaderResource(_pShader)))
		return E_FAIL;


	return S_OK;
}

_bool CAnimation2D::Play_Animation(_float _fTimeDelta, _bool _bReverse)
{
	if (m_fCurrentFrameTime >= (1 / m_fFramesPerSecond))
	{
		m_fCurrentFrameTime = 0;
		m_iCurrentSubFrame++;
		if (m_iCurrentSubFrame >= (_int)m_SpriteFrames[m_iCurrentFrame].second)
		{
			m_iCurrentSubFrame = 0;
			if (_bReverse)
				m_iCurrentFrame--;
			else
				m_iCurrentFrame++;

		}
	}

	if ((_bReverse && m_iCurrentFrame <= 0)
		|| (false == _bReverse && (_int)m_iCurrentFrame >= (_int)(m_iFrameCount)))
	{
		//루프면 처음으로
		if (m_bLoop)
		{
			Reset(_bReverse);
		}
		//루프 아니면 마지막인채로 유지
		else
		{
			m_iCurrentFrame = _bReverse ? 0 : m_iFrameCount - 1;
			return true;
		}
	}

	m_fCurrentFrameTime += _fTimeDelta * m_fSpeedMagnifier;


	return false;
}

_float CAnimation2D::Get_Progress()
{
	_float fProgress = 0;
	if (m_iFrameCount > 0)
	{
		fProgress = (_float)(Get_AccumulativeSubFrameCount(m_iCurrentFrame) + m_iCurrentSubFrame) / (_float)Get_AccumulativeSubFrameCount(m_iFrameCount - 1);
	}
	return fProgress;
}

_float CAnimation2D::Get_AnimationTime()
{
	return (_int)Get_AccumulativeSubFrameCount(m_iFrameCount - 1) / m_fFramesPerSecond / m_fSpeedMagnifier;
}

void CAnimation2D::Set_Progress(_float _fProgress, _bool _bReverse)
{
	_int iTotalSubFrameCount = (_int)Get_AccumulativeSubFrameCount(m_iFrameCount-1);
	_int iTargetSubFrame = (_int)(_fProgress * iTotalSubFrameCount);
	_float fRemainTime = m_fFramesPerSecond * iTotalSubFrameCount * _fProgress - (m_fFramesPerSecond * iTargetSubFrame);
	m_fCurrentFrameTime = fRemainTime;
	for (_uint i = 0; i < m_iFrameCount; i++)
	{
		_int iFrameRun = (_int)m_SpriteFrames[i].second;
		if (iTargetSubFrame - iFrameRun < 0)
		{
			m_iCurrentFrame = i;
			m_iCurrentSubFrame = iTargetSubFrame;
			return;
		}
		iTargetSubFrame -= iFrameRun;
	}
}

_uint CAnimation2D::Get_AccumulativeSubFrameCount(_uint _iFrameIndex)
{
	_uint iAccumulativeSubFrames = 0;
	for (_uint i = 0; i < _iFrameIndex; i++)
	{
		iAccumulativeSubFrames += m_SpriteFrames[i].second;
	}
	return iAccumulativeSubFrames;
}

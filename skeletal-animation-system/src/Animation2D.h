#pragma once
#include "Animation.h"
#include "Texture.h"

BEGIN(Engine)
//애니메이션 내 하나의 프레임 데이터
class ENGINE_DLL CSpriteFrame : public CBase
{
public:
	virtual HRESULT			Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures);

	HRESULT Bind_ShaderResource(class CShader* _pShader);

protected:
	_float2 m_vSpriteStartUV = { 0, 0 };
	_float2 m_vSpriteEndUV = { 0, 0 };
	_float m_fPixelsPerUnrealUnit = { 1.0f };
	_matrix m_matSpriteTransform;
	CTexture* m_pTexture = { nullptr };
public:
	static CSpriteFrame* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures);
};

//하나의 애니메이션 데이터
class ENGINE_DLL CAnimation2D : public CAnimation
{
public:
	HRESULT			Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures);

	HRESULT Bind_ShaderResource(class CShader* _pShader);
	_bool Play_Animation(_float _fTimeDelta, _bool _bReverse);

	virtual _float	Get_Progress() override;
	virtual _float Get_AnimationTime() override;

	virtual void Set_Progress(_float _fProgress, _bool _bReverse)override;
protected:
	_uint Get_AccumulativeSubFrameCount(_uint _iFrameIndex);

protected:
	string m_strName;
	_float m_fFramesPerSecond = 60;
	_uint m_iFrameCount = 0;

	vector<pair<CSpriteFrame*,_uint>> m_SpriteFrames;

	_float m_fCurrentFrameTime = 0;
	_int m_iCurrentFrame = { 0 };
	_int m_iCurrentSubFrame = { 0 };
};

END
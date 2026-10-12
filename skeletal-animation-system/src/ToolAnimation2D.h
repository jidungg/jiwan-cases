#pragma once
#include "Animation2D.h"

BEGIN(AnimTool)
class CToolSpriteFrame :
	public CSpriteFrame
{
public:
	HRESULT			Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, json& _jData, const filesystem::path& _RootDir, map<string, CTexture*>& _Textures);
	virtual HRESULT			Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures);

	HRESULT			Export(ofstream& _outfile);
	HRESULT			Copy_Textures(const filesystem::path& _szDestPath);

private:
	filesystem::path m_pathFinalTexturePath = "";
	vector<_float4> vBakedRenderDatas;
public:
	static CToolSpriteFrame* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, json& _jData, const filesystem::path& _RootDir, map<string, CTexture*>& _Textures);
	static CToolSpriteFrame* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures);
};
class CToolAnimation2D :
	public CAnimation2D
{
public:
	HRESULT			Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, json& _jData, map<string, json>& _jPaperSprites, const filesystem::path& _RootDir, map<string, CTexture*>& _Textures);
	HRESULT			Initialize(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext, const _char* _szDirPath, ifstream& _inFile, map<string, CTexture*>& _Textures);


	HRESULT			Export(ofstream& _outfile);
	HRESULT			Copy_Textures(const filesystem::path& _szDestPath);
	//Get
	_float					Get_SpeedMagnifier() { return m_fSpeedMagnifier; }
	_bool					Is_LoopAnim() { return m_bLoop; }
	string					Get_Name() { return m_strName; }
};

END
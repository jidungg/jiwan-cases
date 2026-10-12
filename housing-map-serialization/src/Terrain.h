#pragma once

#include "TerrainObject.h"

BEGIN(Client)

class CTerrainObject;
class TerrainObjectFactory
{
public:
	static CTerrainObject* Create(
		BUILD_ITEM_TYPE type,
		CGameInstance* pGI,
		CTerrainObject::TERRAINOBJ_DESC& desc);
};
class CTerrain final : public CGameObject
{
public:
	HRESULT Add_TerrainObject(CTerrainObject::TERRAINOBJ_DESC& tDesc);
	HRESULT Remove_TerrainObject(_uint Index);
	_vector IndexToPos(_uint Index);
	_uint PosToIndex(const _float4& Pos);
	_int PosToIndex(const _fvector& Pos);
	_vector SnapPosition(_vector Pos);
	XMUINT3 SplitIndex(_uint iIdx);
	_uint CombineIndex(XMUINT3 i3Idx);
	HRESULT Save_To_Json(string strNewFilepath);
	HRESULT Load_From_Json(string strJsonFilePath);

	_bool Is_Buildable(_vector Pos);
	_bool Is_InSide(_vector Pos);
	_bool Is_ValidIndex(_uint iIndex);
	_bool Is_ValidIndex(XMUINT3 i3Index);
	XMUINT3 Get_Size() { return m_vSize; }
	CTerrainObject* Get_TerrainObject(_uint Index);

private:
	XMUINT3 m_vSize = { 1, 1, 1 };

	vector<CTerrainObject*> m_vecCubes;
};

END
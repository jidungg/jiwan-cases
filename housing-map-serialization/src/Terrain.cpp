#include "stdafx.h"
#include "..\Public\Terrain.h"

#include "GameInstance.h"
#include "JsonParser.h"
#include "ItemDataBase.h"
#include "MonsterSpawner.h"
#include "NPCSpanwer.h"
#include "PortalTerrainObject.h"

namespace
{
	constexpr _float fTerrainObjRotationPerSec = 5.f;
	constexpr _float fTerrainObjSpeedPerSec = 1.f;
}

HRESULT CTerrain::Load_From_Json(string strJsonFilePath)
{
	CItemDataBase* pDB = ITEMDB;
	json j;
	if (FAILED(CJsonParser::ReadJsonFile(strJsonFilePath.c_str(), &j)))
		return E_FAIL;

	m_vSize = { j["size"][0],j["size"][1],j["size"][2] };
	m_vecCubes.resize(m_vSize.x * m_vSize.y * m_vSize.z, nullptr);
	for (const auto& item : j["cells"]) {
		_int eId =  item["ItemId"];
		size_t iteration = item["Iteration"];
		_uint terrIdx = item["Index"];
		BUILD_ITEM_DATA* pBuildItemDesc = static_cast<BUILD_ITEM_DATA*>(pDB->Get_Data(ITEM_TYPE::BUILD, (_uint)eId));
		if (nullptr == pBuildItemDesc)
			return E_FAIL;

		CTerrainObject::TERRAINOBJ_DESC desc;
		desc.fRotationPerSec = fTerrainObjRotationPerSec;
		desc.fSpeedPerSec = fTerrainObjSpeedPerSec;
		lstrcpyW(desc.szModelProtoName, pBuildItemDesc->strModelTag);
		desc.eModelProtoLevelID = LEVEL_LOADING;
		desc.iID = eId;
		desc.direction = item["Direction"];
		desc.vecIData = item["IntData"].get<vector<_int>>();
		desc.vecFData = item["FloatData"].get<vector<_float>>();

		for (int i = 0 ; i < iteration; i++)
		{
			desc.index = terrIdx + i;
			desc.iParentIndex = desc.index;
			XMStoreFloat4(&desc.pos, IndexToPos(desc.index));
			if (FAILED(Add_TerrainObject(desc)))
				return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CTerrain::Save_To_Json(string strNewFilepath)
{
	json j;
	j["cells"] = json::array();
	json& jCells = j["cells"];
	json jCurObj;
	bool bFirst = true;
	for (auto& cell : m_vecCubes)
	{
		if (cell == nullptr) continue;
		CTerrainObject* pTerrainObj = static_cast<CTerrainObject*>(cell);
		if (bFirst)
		{
			jCurObj = pTerrainObj->ToJson();
			bFirst = false;
			continue;
		}
		json jTemp = pTerrainObj->ToJson();
		int curIdx = jCurObj["Index"];
		int curIter = jCurObj["Iteration"];

		if ( jCurObj["ItemId"] == jTemp["ItemId"]
			&& jCurObj["IntData"] == jTemp["IntData"]
			&& jCurObj["FloatData"] == jTemp["FloatData"]
			&& jCurObj["Direction"] == jTemp["Direction"]
			&& (curIdx +curIter) == jTemp["Index"])
		{
			jCurObj["Iteration"] = jCurObj["Iteration"] + 1;
		}
		else
		{
			jCells.push_back(jCurObj);
			jCurObj = jTemp;
		}

	}
	jCells.push_back(jCurObj);
	j["size"] = { m_vSize.x,m_vSize.y,m_vSize.z };

	CJsonParser::SaveJsonFile(strNewFilepath.c_str(), j);
	return S_OK;
}

_vector CTerrain::IndexToPos(_uint Index)
{
	return XMVectorSet(Index % m_vSize.x, 
		Index / (m_vSize.x * m_vSize.z),
		Index / m_vSize.x % m_vSize.z,
		1);
}

_uint CTerrain::PosToIndex(const _float4& Pos)
{
	_uint x = static_cast<_uint>(roundf(Pos.x));
	_uint y = static_cast<_uint>(floorf(Pos.y)); // 0~1 = 0
	_uint z = static_cast<_uint>(roundf(Pos.z));

	return x + m_vSize.x * z + m_vSize.x * m_vSize.z * y;
}

_int CTerrain::PosToIndex(const _fvector& Pos)
{
	_int x = static_cast<_int>(roundf(XMVectorGetX(Pos)));
	_int y = static_cast<_int>(floorf(XMVectorGetY(Pos))); // 0~1 = 0
	_int z = static_cast<_int>(roundf(XMVectorGetZ(Pos)));
	if (x < 0 || x >= (_int)m_vSize.x)
		return -1;
	if (y < 0 || y >= (_int)m_vSize.y)
		return -1;
	if (z < 0 || z >= (_int)m_vSize.z)
		return -1;
	return x + m_vSize.x * z + m_vSize.x * m_vSize.z * y;
}

_vector CTerrain::SnapPosition(_vector Pos)
{
	Pos.m128_f32[0] = roundf(Pos.m128_f32[0]);
	Pos.m128_f32[1] = floorf(Pos.m128_f32[1]);
	Pos.m128_f32[2] = roundf(Pos.m128_f32[2]);
	return Pos;
}

XMUINT3 CTerrain::SplitIndex(_uint iIdx)
{
	return XMUINT3{ iIdx % m_vSize.x, iIdx / (m_vSize.x * m_vSize.z), iIdx / m_vSize.x % m_vSize.z, };
}

_uint CTerrain::CombineIndex(XMUINT3 i3Idx)
{
	return i3Idx.x + m_vSize.x * i3Idx.z + m_vSize.x * m_vSize.z * i3Idx.y;
}

HRESULT CTerrain::Add_TerrainObject(CTerrainObject::TERRAINOBJ_DESC& tDesc)
{
	if (false == Is_ValidIndex(SplitIndex(tDesc.index)))
		return E_FAIL;
	if (m_vecCubes[tDesc.index] != nullptr)
		return E_FAIL;
	tDesc.pCubeTerrain = this;
	BUILD_ITEM_DATA* pBuildItemDesc = static_cast<BUILD_ITEM_DATA*>(ITEMDB->Get_Data(ITEM_TYPE::BUILD, (_uint)tDesc.iID));
	if (nullptr == pBuildItemDesc)
		return E_FAIL;
	BUILD_ITEM_TYPE eType = pBuildItemDesc->eBuildType;

	CTerrainObject* pGameObject = TerrainObjectFactory::Create(eType, m_pGameInstance, tDesc);
	if (nullptr == pGameObject)
		return E_FAIL;
	Add_Child(pGameObject);
	m_vecCubes[tDesc.index] = pGameObject;
	return S_OK;
}

HRESULT CTerrain::Remove_TerrainObject(_uint Index)
{
	if (Index >= (_uint)m_vecCubes.size())
		return E_FAIL;
	if (m_vecCubes[Index] == nullptr)
		return E_FAIL;
	m_vecCubes[Index]->Set_Dead();
	Safe_Release(m_vecCubes[Index]);

	return S_OK;
}

CTerrainObject* CTerrain::Get_TerrainObject(_uint Index)
{
	if (Index < (_uint)m_vecCubes.size())
		return m_vecCubes[Index];
	return nullptr;
}

_bool CTerrain::Is_Buildable(_vector Pos)
{
	if (false == Is_InSide(Pos))
		return false;
	_int iIdx = PosToIndex(Pos);
	if (false  == Is_ValidIndex(iIdx))
		return false;
	if (m_vecCubes[iIdx] != nullptr)
		return false;

	return true;
}

_bool CTerrain::Is_InSide(_vector vPos)
{
	_float4 fPos;
	XMStoreFloat4(&fPos, vPos);
	if (fPos.x < -0.5 || fPos.x >= m_vSize.x - 0.5)
		return false;
	if (fPos.z < -0.5 || fPos.z >= m_vSize.z - 0.5)
		return false;
	if (fPos.y < 0 || fPos.y >= m_vSize.y )
		return false;
	return true;
}

_bool CTerrain::Is_ValidIndex(_uint iIndex)
{
	return iIndex >= 0 && iIndex < m_vecCubes.size();
}

_bool CTerrain::Is_ValidIndex(XMUINT3 i3Index)
{
	if (i3Index.x < 0 || i3Index.x >= m_vSize.x)
		return false;
	if (i3Index.y < 0 || i3Index.y >= m_vSize.y)
		return false;
	if (i3Index.z < 0 || i3Index.z >= m_vSize.z)
		return false;
	return true;
}

CTerrainObject* TerrainObjectFactory::Create(BUILD_ITEM_TYPE eType, CGameInstance* pGI, CTerrainObject::TERRAINOBJ_DESC& tDesc)
{
	CTerrainObject* pGameObject = nullptr;
	switch (eType)
	{
	case Client::BUILD_ITEM_TYPE::GROUND:
	case Client::BUILD_ITEM_TYPE::CUBRIC:
	case Client::BUILD_ITEM_TYPE::FUNCT:
	case Client::BUILD_ITEM_TYPE::DEFORM:
	case Client::BUILD_ITEM_TYPE::FLOOR:
	case Client::BUILD_ITEM_TYPE::WALL:
	case Client::BUILD_ITEM_TYPE::NATURE:
	case Client::BUILD_ITEM_TYPE::PROP:
	case Client::BUILD_ITEM_TYPE::STRUC:
		pGameObject = static_cast<CTerrainObject*>(pGI->Clone_Proto_Object_Stock(CTerrainObject::m_szProtoTag, &tDesc));
		break;
	case Client::BUILD_ITEM_TYPE::MONSTER_SPAWNER:
		pGameObject = static_cast<CTerrainObject*>(pGI->Clone_Proto_Object_Stock(CMonsterSpawner::m_szProtoTag, &tDesc));
		break;
	case Client::BUILD_ITEM_TYPE::PLAYER_SPAWNER:
		break;
	case Client::BUILD_ITEM_TYPE::PORTAL:
		pGameObject = static_cast<CTerrainObject*>(pGI->Clone_Proto_Object_Stock(CPortalTerrainObject::m_szProtoTag, &tDesc));
		break;
	case Client::BUILD_ITEM_TYPE::NPC_SPAWNER:
		pGameObject = static_cast<CTerrainObject*>(pGI->Clone_Proto_Object_Stock(CNPCSpanwer::m_szProtoTag, &tDesc));
		break;
	case Client::BUILD_ITEM_TYPE::LAST:
	default:
		break;
	}

	return pGameObject;
}

#pragma once

#include "TerrainObject.h"

BEGIN(Client)

class CTerrainObject;
class CTerrain final : public CGameObject
{
public:
	_bool RayCast(const Ray& tRay, RaycastHit* pOut);
	_int PosToIndex(const _fvector& Pos);

private:
	XMUINT3 m_vSize = { 1, 1, 1 };

	vector<CTerrainObject*> m_vecCubes;
};

END
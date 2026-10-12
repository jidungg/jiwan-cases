#pragma once
#include "TerrainObject.h"

BEGIN(Client)
class CBuildPreview :
	public CTerrainObject
{
public:
	static constexpr _tchar m_szProtoTag[] = L"Prototype_GameObject_BuildPreview";
public:
	virtual void Compute_Matrix();
};

END
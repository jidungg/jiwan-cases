#include "stdafx.h"
#include "..\Public\Terrain.h"

_bool CTerrain::RayCast(const Ray& tRay, RaycastHit* pOut)
{
	_vector vOrigin = tRay.vOrigin;
	_vector vDir = XMVector3Normalize(tRay.vDirection);
	//dir 벡터가 0인 경우 예외 처리
	if (XMVector3Equal(vDir, XMVectorZero()))
		return false;

	// X/Z 중심 보정
	_vector vHalfCellXZ = XMVectorSet(0.5f, 0.f, 0.5f, 0.f);
	_vector vPos = vOrigin + vHalfCellXZ;

	// 현재 셀 좌표
	_int iX = (_int)(XMVectorGetX(vPos));
	_int iY = (_int)(XMVectorGetY(vPos));
	_int iZ = (_int)(XMVectorGetZ(vPos));

	_float fDirX = XMVectorGetX(vDir);
	_float fDirY = XMVectorGetY(vDir);
	_float fDirZ = XMVectorGetZ(vDir);

	//셀 인덱스 증가 방향
	_int iStepX = (fDirX > 0) ? 1 : (fDirX < 0 ? -1 : 0);
	_int iStepY = (fDirY > 0) ? 1 : (fDirY < 0 ? -1 : 0);
	_int iStepZ = (fDirZ > 0) ? 1 : (fDirZ < 0 ? -1 : 0);

	//셀 1칸 이동하는데 걸리는 t값
	_float fTDeltaX = (fDirX != 0) ? fabsf(1.f / fDirX) : FLT_MAX;
	_float fTDeltaY = (fDirY != 0) ? fabsf(1.f / fDirY) : FLT_MAX;
	_float fTDeltaZ = (fDirZ != 0) ? fabsf(1.f / fDirZ) : FLT_MAX;

	_float fNextBoundaryX = iX + (iStepX > 0 ? 0.5f : -0.5f);
	_float fNextBoundaryY = iY + (iStepY > 0 ? 1.f  : 0.f);
	_float fNextBoundaryZ = iZ + (iStepZ > 0 ? 0.5f : -0.5f);

	//다음 경계면에서의 t값
	_float fTMaxX = (fDirX != 0) ? (fNextBoundaryX - XMVectorGetX(vPos)) / fDirX : FLT_MAX;
	_float fTMaxY = (fDirY != 0) ? (fNextBoundaryY - XMVectorGetY(vPos)) / fDirY : FLT_MAX;
	_float fTMaxZ = (fDirZ != 0) ? (fNextBoundaryZ - XMVectorGetZ(vPos)) / fDirZ : FLT_MAX;

	//현재 Ray 파라미터
	_float fT = 0.f;

	while (fT <= tRay.fDist)
	{
		_vector vRealPos = XMVectorSet(iX, iY, iZ, 0.f) - vHalfCellXZ;

		_int iIdx = PosToIndex(vRealPos);
		if (iIdx < 0)
			return false;
		if (m_vecCubes[iIdx] && m_vecCubes[iIdx]->RayCast(tRay, pOut))
			return true;

		// DDA step
		if (fTMaxX < fTMaxY && fTMaxX < fTMaxZ)
		{
			iX += iStepX;
			fT = fTMaxX;
			fTMaxX += fTDeltaX;
		}
		else if (fTMaxZ < fTMaxY)
		{
			iZ += iStepZ;
			fT = fTMaxZ;
			fTMaxZ += fTDeltaZ;
		}
		else
		{
			iY += iStepY;
			fT = fTMaxY;
			fTMaxY += fTDeltaY;
		}
	}
	return false;
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

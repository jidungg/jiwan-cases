#include "stdafx.h"
#include "BuildPreview.h"

void CBuildPreview::Compute_Matrix()
{
	_matrix pParentMatrix = XMMatrixTranslationFromVector(XMLoadFloat4x4(m_pParentMatrix).r[CTransform::STATE_POSITION]);
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * pParentMatrix);

}

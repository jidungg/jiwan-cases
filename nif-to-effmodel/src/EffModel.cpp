#include "stdafx.h"
#include "EffModel.h"
#include "EffBone.h"
#include "EffMesh.h"
#include "EffMaterialProperty.h"
#include "EffTexturingProperty.h"
#include "EffConcreteController.h"
#include "GameInstance.h"
#include "EffFlipController.h"


HRESULT CEffModel::Initialize_Prototype(const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
    std::ifstream inFile(pModelFilePath, std::ios::binary);
    if (!inFile) {
        string str = "파일을 열 수 없습니다.";
        str += pModelFilePath;
        MessageBoxA(NULL, str.c_str(), "에러", MB_OK);
        return E_FAIL;
    }
    bool bAnim;
    inFile.read(reinterpret_cast<char*>(&bAnim), sizeof(bool));

    if (FAILED(Ready_Bones(inFile, -1)))
        return E_FAIL;

    if (FAILED(Ready_Meshes(inFile)))
        return E_FAIL;

    if (FAILED(Ready_Materials(inFile)))
        return E_FAIL;

    if (FAILED(Ready_Textures(inFile, pModelFilePath)))
        return E_FAIL;

    if (FAILED(Ready_Texturings(inFile)))
        return E_FAIL;

    if (FAILED(Ready_Controls(inFile)))
        return E_FAIL;

    inFile.close();

    return S_OK;
}

HRESULT CEffModel::Ready_Bones(ifstream& inFile, _uint iParentBoneIndex)
{
    CEffBone* pBone;

    pBone = CEffBone::Create(inFile, iParentBoneIndex);

    if (nullptr == pBone)
        return E_FAIL;

    m_vecBone.push_back(pBone);

    iParentBoneIndex = (_uint)m_vecBone.size() - 1;
    _uint iNumChildren = 0;
    inFile.read(reinterpret_cast<char*>(&iNumChildren), sizeof(_uint));
    for (_uint i = 0; i < iNumChildren; ++i)
    {
        Ready_Bones(inFile, iParentBoneIndex);
    }

    return S_OK;
}

HRESULT CEffModel::Ready_Meshes(ifstream& inFile)
{
    inFile.read(reinterpret_cast<char*>(&m_iNumMeshes), sizeof(_uint));
    for (_uint i = 0; i < m_iNumMeshes; i++)
    {
        CEffMesh* pMesh = CEffMesh::Create(m_pDevice, m_pContext, this, inFile, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;

        m_vecMesh.push_back(pMesh);
    }

    return S_OK;
}

HRESULT CEffModel::Ready_Materials(ifstream& inFile)
{
    _uint iNumMaterials = 0;
    inFile.read(reinterpret_cast<char*>(&iNumMaterials), sizeof(_uint));

    m_vecMaterial.resize(iNumMaterials);

    for (_uint i = 0; i < iNumMaterials; i++)
    {
        CEffMaterialProperty* pMaterial = CEffMaterialProperty::Create(m_pDevice, m_pContext, inFile);
        if (nullptr == pMaterial)
            return E_FAIL;

        m_vecMaterial[i] = pMaterial;
    }

    return S_OK;
}

HRESULT CEffModel::Ready_Textures(ifstream& inFile, const _char* pModelFilePath)
{
    _uint iNumTextures = 0;
    inFile.read(reinterpret_cast<char*>(&iNumTextures), sizeof(_uint));
    m_vecTexture.resize(iNumTextures);

    _char szDrive[MAX_PATH] = "";
    _char szDirectory[MAX_PATH] = "";
    _splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDirectory, MAX_PATH, nullptr, 0, nullptr, 0);
    strcat_s(szDrive, szDirectory);

    for (_uint i = 0; i < iNumTextures; i++)
    {
        CTexture* pTexture = CTexture::Create(m_pDevice, m_pContext, szDrive, inFile);
        if (nullptr == pTexture)
            return E_FAIL;

        m_vecTexture[i] = pTexture;
    }
    return S_OK;
}

HRESULT CEffModel::Ready_Texturings(ifstream& inFile)
{
    _uint iNumTexturings = 0;
    inFile.read(reinterpret_cast<char*>(&iNumTexturings), sizeof(_uint));
    m_vecTexturing.resize(iNumTexturings);
    for (_uint i = 0; i < iNumTexturings; i++)
    {
        CEffTexturingProperty* pTexturing = CEffTexturingProperty::Create(m_pDevice, m_pContext, inFile);
        if (nullptr == pTexturing)
            return E_FAIL;

        m_vecTexturing[i] = pTexturing;
    }

    return S_OK;
}


HRESULT CEffModel::Ready_Controls(ifstream& inFile)
{
    _uint iNumControls = 0;
    inFile.read(reinterpret_cast<char*>(&iNumControls), sizeof(_uint));
    m_vecControl.resize(iNumControls);
    for (_uint i = 0; i < iNumControls; i++)
    {
        EFF_CTRL_TYPE eCtrlType;
        inFile.read(reinterpret_cast<char*>(&eCtrlType), sizeof(_uint));
        CEffController* pController = nullptr;
        switch (eCtrlType)
        {
        case Client::CT_TRANSFORM:
            pController = CEffTransformController::Create(inFile, this);
            break;
        case Client::CT_ALPHA:
            pController = CEffAlphaController::Create(inFile, this);
            break;
        case Client::CT_MATERIAL_COLOR:
            pController = CEffMaterialColorController::Create(inFile, this);
            break;
        case Client::CT_TEXTURE_TRANSFORM:
            pController = CEffTextureTransformController::Create(inFile, this);
            break;
        case Client::CT_TEXTURE_FLIP:
            pController = CEffFlipController::Create(inFile, this);
            break;
        case Client::CT_LAST:
        default:
            break;
        }
        if (nullptr == pController)
            return E_FAIL;
        m_vecControl[i] = pController;
    }
    return S_OK;
}

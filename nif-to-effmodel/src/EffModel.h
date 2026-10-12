#pragma once
#include "Model.h"

BEGIN(Engine)
class CTexture;
END

BEGIN(Client)
class CEffModel : public CComponent
{
public:
    virtual HRESULT Initialize_Prototype(const _char* pModelFilePath, _fmatrix PreTransformMatrix);

protected:
    HRESULT Ready_Bones(ifstream& inFile, _uint iParentBoneIndex);
    HRESULT Ready_Meshes(ifstream& inFile);
    HRESULT Ready_Materials(ifstream& inFile);
    HRESULT Ready_Textures(ifstream& inFile, const _char* pModelFilePath);
    HRESULT Ready_Texturings(ifstream& inFile);
    HRESULT Ready_Controls(ifstream& inFile);

private:

    vector<class CEffBone*> m_vecBone;

    _uint m_iNumMeshes = { 0 };
    vector<class CEffMesh*> m_vecMesh;

    vector<class CEffMaterialProperty*> m_vecMaterial;

    vector<CTexture*> m_vecTexture;

    vector<class CEffTexturingProperty*> m_vecTexturing;

    vector<class CEffController*> m_vecControl;

    _float4x4 m_PreTransformMatrix = {};
};

END
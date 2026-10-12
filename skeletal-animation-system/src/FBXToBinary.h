#pragma once

typedef struct tagKeyFrame
{
	float	 fTrackPosition; // 애니메이션 트랙위에서의 포지션을 정의.
	XMFLOAT3 vScale;
	XMFLOAT4 vRotation; // 쿼터니언을 사용하기 위해서.
	XMFLOAT3 vPosition;
}KEYFRAME;

class CFbxToBinary
{
public:
	HRESULT FbxToBinary(const string& inFilePath);

private:
	HRESULT Write_Meshe(const aiMesh* pAIMesh, ofstream& outFile);
	HRESULT Write_Material(const aiMaterial* pAIMaterial, ofstream& outFile);
	HRESULT Write_Bone(const aiNode* pAINode, ofstream& outFile);
	HRESULT Write_Animation(const aiAnimation* pAIAnim, ofstream& outFile);
private:
	Assimp::Importer			m_Importer;
	const aiScene* m_pAIScene = { nullptr };
	bool m_bAnim = { false };
};

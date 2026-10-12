#include "pch.h"

#include "FBXToBinary.h"

HRESULT CFbxToBinary::FbxToBinary(const string& inFilePath)
{
	_uint		iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };


	m_pAIScene = m_Importer.ReadFile(inFilePath, iFlag);

	m_bAnim = m_pAIScene->HasAnimations();

	if (m_bAnim == false)
	{

		iFlag |= aiProcess_PreTransformVertices;

		m_pAIScene = m_Importer.ReadFile(inFilePath, iFlag);
	}

	if (0 == m_pAIScene)
		return E_FAIL;

	_char		szDrive[MAX_PATH] = "";
	_char		szDirectory[MAX_PATH] = "";
	_char		szFileName[MAX_PATH] = "";
	_splitpath_s(inFilePath.c_str(), szDrive, MAX_PATH, szDirectory, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);
	_char		szNewFileName[MAX_PATH] = "";
	strcpy_s(szNewFileName, szDrive);
	strcat_s(szNewFileName, szDirectory);
	strcat_s(szNewFileName, szFileName);
	strcat_s(szNewFileName, ".model");
	std::ofstream outFile(szNewFileName, std::ios::binary);
	if (!outFile) {
		std::cerr << "파일 열기 실패." << std::endl;
	}
	outFile.write(reinterpret_cast<const char*>(&m_bAnim), 1);

	if (FAILED(Write_Bone(m_pAIScene->mRootNode, outFile)))
		return E_FAIL;

	outFile.write(reinterpret_cast<const char*>(&m_pAIScene->mNumMeshes), sizeof(_uint));
	for (size_t meshIdx = 0; meshIdx < m_pAIScene->mNumMeshes; meshIdx++)
	{
		if (FAILED(Write_Meshe(m_pAIScene->mMeshes[meshIdx], outFile)))
			return E_FAIL;

	}

	_uint iNumMaterials = m_pAIScene->mNumMaterials;
	outFile.write(reinterpret_cast<const char*>(&iNumMaterials), sizeof(_uint));
	for (size_t i = 0; i < iNumMaterials; i++)
	{
		if (FAILED(Write_Material(m_pAIScene->mMaterials[i], outFile)))
			return E_FAIL;
	}

	outFile.write(reinterpret_cast<const char*>(&m_pAIScene->mNumAnimations), sizeof(_uint));
	for (size_t i = 0; i < m_pAIScene->mNumAnimations; i++)
	{
		if (FAILED(Write_Animation(m_pAIScene->mAnimations[i], outFile)))
			return E_FAIL;
	}

	outFile.close();
	return S_OK;
}

HRESULT CFbxToBinary::Write_Meshe(const aiMesh* pAIMesh, ofstream& outFile)
{
	outFile.write(reinterpret_cast<const char*>(&pAIMesh->mMaterialIndex), sizeof(_uint));
	outFile.write(reinterpret_cast<const char*>(&pAIMesh->mName.length), sizeof(_uint));
	outFile.write(pAIMesh->mName.data, pAIMesh->mName.length);
	outFile.write(reinterpret_cast<const char*>(&pAIMesh->mNumVertices), sizeof(_uint));
	outFile.write(reinterpret_cast<const char*>(&pAIMesh->mNumFaces), sizeof(_uint));

	for (size_t i = 0; i < pAIMesh->mNumVertices; i++)
	{
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mVertices[i]), sizeof(_float3));
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mNormals[i]), sizeof(_float3));
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mTextureCoords[0][i]), sizeof(_float2));

		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mTangents[i]), sizeof(_float3));
	}
	if (m_bAnim)
	{
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mNumBones), sizeof(_uint));

		for (size_t i = 0; i < pAIMesh->mNumBones; i++)
		{
			aiBone* pAIBone = pAIMesh->mBones[i];

			outFile.write(reinterpret_cast<const char*>(&pAIBone->mName.length), sizeof(_uint));
			outFile.write(pAIBone->mName.data, pAIBone->mName.length);

			outFile.write(reinterpret_cast<const char*>(&pAIBone->mOffsetMatrix), sizeof(_float4x4));

			outFile.write(reinterpret_cast<const char*>(&pAIBone->mNumWeights), sizeof(_uint));
			for (size_t j = 0; j < pAIBone->mNumWeights; j++)
			{
				outFile.write(reinterpret_cast<const char*>(&pAIBone->mWeights[j].mVertexId), sizeof(_uint));
				outFile.write(reinterpret_cast<const char*>(&pAIBone->mWeights[j].mWeight), sizeof(_float));
			}

		}

	}


	for (size_t i = 0; i < pAIMesh->mNumFaces; i++)
	{
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mFaces[i].mIndices[0]), sizeof(_uint));
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mFaces[i].mIndices[1]), sizeof(_uint));
		outFile.write(reinterpret_cast<const char*>(&pAIMesh->mFaces[i].mIndices[2]), sizeof(_uint));
	}
	return S_OK;
}
//55, 44
HRESULT CFbxToBinary::Write_Material(const aiMaterial* pAIMaterial, ofstream& outFile)
{
	for (_uint texTypeIdx = aiTextureType_DIFFUSE; texTypeIdx < aiTextureType_UNKNOWN; texTypeIdx++)
	{
		_uint numTextures = pAIMaterial->GetTextureCount(aiTextureType(texTypeIdx));
		outFile.write(reinterpret_cast<const char*>(&numTextures), sizeof(_uint));
		for (_uint i = 0; i < numTextures; i++)
		{
			aiString strPath;
			pAIMaterial->GetTexture(aiTextureType(texTypeIdx), i, &strPath);
			outFile.write(reinterpret_cast<const char*>(&strPath.length), sizeof(ai_uint32));
			outFile.write(strPath.C_Str(), strPath.length);
		}

	}

	return S_OK;
}

HRESULT CFbxToBinary::Write_Bone(const aiNode* pAINode, ofstream& outFile)
{

	outFile.write(reinterpret_cast<const char*>(&pAINode->mName.length), sizeof(_uint));
	outFile.write(pAINode->mName.data, pAINode->mName.length);

	outFile.write(reinterpret_cast<const char*>(&pAINode->mTransformation), sizeof(_float4x4));
	outFile.write(reinterpret_cast<const char*>(&pAINode->mNumChildren), sizeof(_uint));
	for (size_t i = 0; i < pAINode->mNumChildren; ++i)
	{
		Write_Bone(pAINode->mChildren[i], outFile);
	}
	return S_OK;
}

HRESULT CFbxToBinary::Write_Animation(const aiAnimation* pAIAnim, ofstream& outFile)
{
	outFile.write(reinterpret_cast<const char*>(&pAIAnim->mName.length), sizeof(_uint));
	outFile.write(pAIAnim->mName.data, pAIAnim->mName.length);

	outFile.write(reinterpret_cast<const char*>(&pAIAnim->mDuration), sizeof(double));
	outFile.write(reinterpret_cast<const char*>(&pAIAnim->mTicksPerSecond), sizeof(double));

	//추가됨
	_bool bLoop = false;
	outFile.write(reinterpret_cast<char*>(&bLoop), sizeof(_bool));
	//추가됨
	_float fSpeedMagnifier = 1.0f;
	outFile.write(reinterpret_cast<char*>(&fSpeedMagnifier), sizeof(_float));

	outFile.write(reinterpret_cast<const char*>(&pAIAnim->mNumChannels), sizeof(_uint));

	for (size_t i = 0; i < pAIAnim->mNumChannels; i++)
	{
		aiNodeAnim* pAIChannel = pAIAnim->mChannels[i];
		outFile.write(reinterpret_cast<const char*>(&pAIChannel->mNodeName.length), sizeof(_uint));
		outFile.write(pAIChannel->mNodeName.data, pAIChannel->mNodeName.length);

		_uint m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);

		m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);
		outFile.write(reinterpret_cast<const char*>(&m_iNumKeyFrames), sizeof(_uint));

		_float3			vScale, vPosition;
		_float4			vRotation;
		for (size_t i = 0; i < m_iNumKeyFrames; i++)
		{
			KEYFRAME		KeyFrame = {};

			if (i < pAIChannel->mNumScalingKeys)
			{
				memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
				KeyFrame.fTrackPosition = (_float)pAIChannel->mScalingKeys[i].mTime;
			}
			if (i < pAIChannel->mNumRotationKeys)
			{
				vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
				vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
				vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
				vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;
				KeyFrame.fTrackPosition = (_float)pAIChannel->mRotationKeys[i].mTime;
			}
			if (i < pAIChannel->mNumPositionKeys)
			{
				memcpy(&vPosition, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
				KeyFrame.fTrackPosition =(_float) pAIChannel->mPositionKeys[i].mTime;
			}

			KeyFrame.vScale = vScale;
			KeyFrame.vRotation = vRotation;
			KeyFrame.vPosition = vPosition;

			outFile.write(reinterpret_cast<const char*>(&KeyFrame), sizeof(KEYFRAME));

		}


	}
	return S_OK;
}


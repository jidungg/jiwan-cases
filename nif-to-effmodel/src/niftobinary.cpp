#include "niftobinary.h"

#include <QBuffer>
#include <QByteArray>

#include "src/model/nifmodel.h"
#include "src/data/niftypes.h"
#include "io/nifstream.h"

enum TEX_TYPE
{
    TT_BASE,
    TT_DARK,
    TT_DETAIL,
    TT_GLOSS,
    TT_GLOW,
    TT_BUMPMAP,
    TT_NORMAL,
    TT_PARALLAX,
    TT_DECAL0,
    TT_DECAL1,
    TT_DECAL2,
    TT_DECAL3,
    TT_LAST
};
enum CTRL_TYPE
{
    CT_TRANSFORM,
    CT_ALPHA,
    CT_MATERIAL_COLOR,
    CT_TEXTURE_TRANSFORM,
    CT_TEXTURE_FLIP,
    CT_LAST
};
enum TEX_OPERATION_TYPE
{
    TO_TRANSLATE_U,
    TO_TRANSLATE_V,
    TO_ROTATE,
    TO_SCALE_U,
    TO_SCALE_V,
    TO_LAST
};

const char * texnames[TT_LAST] = {
    "Base Texture", "Dark Texture", "Detail Texture", "Gloss Texture", "Glow Texture", "Bump Map Texture", "Normal Texture","Parallax Texture","Decal 0 Texture","Decal 1 Texture","Decal 2 Texture","Decal 3 Texture"
};

using namespace std;

namespace
{
    // NiMesh::DataStreamFormat bit fields
    constexpr int typeLengthMask = 0x000F0000;
    constexpr int typeLengthShift = 0x10;
    constexpr int typeCodeMask = 0x00000FF0;
    constexpr int typeCodeShift = 0x04;

    constexpr quint32 indexLimit = 0xFFFF;
    constexpr float maxGlossiness = 128.0f;
}

map<QModelIndex, unsigned int> mapBoneIndex;
map<QModelIndex, unsigned int> mapMeshIndex;
map<QString, unsigned int> mapMaterialIndex;
map<QModelIndex, unsigned int> mapTextureIndex;
map<QModelIndex, unsigned int> mapTexturingIndex;
map<QModelIndex, unsigned int> mapControllerIndex;
int iBoneIndex = 0;
void MakeBoneList( NifModel* nifModel,int iParentLink)
{
    if(iParentLink == -1)
    {
        mapBoneIndex.clear();
        iBoneIndex = 0;
        int blockCount = nifModel->getBlockCount() ;
        for (int i = 0; i < blockCount; ++i) {
           QModelIndex iBlock = nifModel->getBlock(i);
           if (nifModel->inherits( iBlock, "NiNode" ) || nifModel->inherits( iBlock, "NiMesh" ))
           {
               if(mapBoneIndex.find(iBlock) != mapBoneIndex.end())
                   continue;
               mapBoneIndex[iBlock] = iBoneIndex++;
               MakeBoneList(nifModel,i);
           }
        }
    }
    else
    {
        QList<int> liChildLinks = nifModel->getChildLinks(iParentLink);
        for(int iLink : liChildLinks)
        {
            QModelIndex iCHildBlockIdx = nifModel->getBlock(iLink);
            if (nifModel->inherits( iCHildBlockIdx, "NiNode" )||nifModel->inherits( iCHildBlockIdx, "NiMesh" ) )
            {
                if(mapBoneIndex.find(iCHildBlockIdx) != mapBoneIndex.end())
                    continue;
                mapBoneIndex[iCHildBlockIdx] = iBoneIndex++;
                MakeBoneList(nifModel,iLink);
            }
        }
    }

}

void MakeMeshList( NifModel* nifModel)
{
    mapMeshIndex.clear();
    int blockCount = nifModel->getBlockCount() ;
    unsigned int iMeshIndex = 0;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if(nifModel->itemName(iBlock) == "NiMesh")
       {
           mapMeshIndex[iBlock] = iMeshIndex;
           iMeshIndex++;
       }
    }
}
void MakeTextureList(NifModel* nifModel)
{
    mapTextureIndex.clear();
    int blockCount = nifModel->getBlockCount() ;
    unsigned int iTextureIndex = 0;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if(nifModel->itemName(iBlock) == "NiSourceTexture")
       {
           mapTextureIndex[iBlock] = iTextureIndex;
           iTextureIndex++;
       }
    }
}

void MakeMaterialList( NifModel* nifModel)
{
    mapMaterialIndex.clear();
    int blockCount = nifModel->getBlockCount() ;
    unsigned int iMaterialIndex = 0;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if(nifModel->itemName(iBlock) == "NiMaterialProperty")
       {
           QString materialName = nifModel->get<QString>(iBlock,"Name");
           if(mapMaterialIndex.find(materialName) == mapMaterialIndex.end())
           {
               mapMaterialIndex.insert(make_pair(materialName ,iMaterialIndex) );
               iMaterialIndex++;
           }
       }
    }
}
void MakeTexturingList( NifModel* nifModel)
{
    mapTexturingIndex.clear();
    int blockCount = nifModel->getBlockCount() ;
    unsigned int iTextureingIndex = 0;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if(nifModel->inherits(iBlock , "NiTexturingProperty"))
       {
           mapTexturingIndex[iBlock] = iTextureingIndex;
           iTextureingIndex++;
       }
    }
}
void MakeControllerList( NifModel* nifModel)
{
    mapControllerIndex.clear();
    int blockCount = nifModel->getBlockCount() ;
    unsigned int iControllerIndex = 0;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if(nifModel->itemName(iBlock) == "NiTransformController"
               ||nifModel->itemName(iBlock) == "NiTextureTransformController"
               || nifModel->itemName(iBlock) == "NiAlphaController"
               ||nifModel->itemName(iBlock) == "NiMaterialColorController"
               ||nifModel->itemName(iBlock) == "NiFlipController")
       {
           mapControllerIndex[iBlock] = iControllerIndex;
           iControllerIndex++;
       }
    }
}

//============================================ BONE================================================

struct BoneData
{
    void update(const NifModel * nifModel, const QModelIndex & iBlockIdx )
    {
        strName = nifModel->get<QString>(iBlockIdx, "Name");

        //Controller
        QModelIndex iCtrl = nifModel->getBlock( nifModel->getLink( iBlockIdx, "Controller" ) );
        while ( iCtrl.isValid() && nifModel->inherits( iCtrl, "NiTimeController" ) )
        {
            if(mapControllerIndex.find(iCtrl) != mapControllerIndex.end())
            {
                vecTransformControllerIndex.push_back( mapControllerIndex[iCtrl]);

            }
            iCtrl = nifModel->getBlock( nifModel->getLink( iCtrl, "Next Controller" ) );
        }

        //Billboard
        if(nifModel->itemName(iBlockIdx) == "NiBillboardNode")
            bBillboard = true;


        //TransformMatrix
        Vector3 vPosition = nifModel->get<Vector3>(iBlockIdx,"Translation");
        Matrix matRotation = nifModel->get<Matrix>(iBlockIdx,"Rotation");
        float fScale = nifModel->get<float>(iBlockIdx,"Scale");
        matTransformMatrix.compose(vPosition, matRotation,Vector3(fScale,fScale,fScale));

        iChildCount = 0;
        QList<int> liChildLinks = nifModel->getChildLinks(nifModel->getBlockNumber(iBlockIdx));
        for(int iLink : liChildLinks)
        {
            QModelIndex iCHildBlockIdx = nifModel->getBlock(iLink);
            if (nifModel->inherits( iCHildBlockIdx, "NiNode" )||nifModel->inherits( iCHildBlockIdx, "NiMesh" ) )
                iChildCount++;
        }

    }

    void ExportToBinary(ofstream& outfile)
    {
        //Name
        unsigned int iLength = strName.length();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        outfile.write(strName.toStdString().c_str(), iLength);

        //billboard
        outfile.write(reinterpret_cast<const char*>(&bBillboard), 1);
        //TransformMatrix
        outfile.write(reinterpret_cast<const char*>(matTransformMatrix.data()), sizeof(float)*16);
        //자식 노드 갯수
        outfile.write(reinterpret_cast<const char*>(&iChildCount), sizeof( uint));
    }

    QString strName;
    QVector<uint> vecTransformControllerIndex;
    bool bBillboard = {false};
    Matrix4 matTransformMatrix;
    uint iChildCount = 0;
};

void ExportBone(NifModel* nifModel, ofstream& outfile) {

    for(auto& bone : mapBoneIndex)
    {
        BoneData  tBoneData;
        tBoneData.update(nifModel,bone.first);
        tBoneData.ExportToBinary(outfile);
    }

}

//============================================ MESH================================================
struct BoneAffectedVertexData
{
    void ExportToBinary(ofstream& outfile)
    {
        outfile.write(reinterpret_cast<const char*>(&iVertexIndex), sizeof(uint));
        outfile.write(reinterpret_cast<const char*>(&fWeight), sizeof(float));
    }
    uint iVertexIndex = {0};
    float fWeight = {1};
};

struct MeshBoneData
{
    void ExportToBinary(ofstream& outfile)
    {
        outfile.write(reinterpret_cast<const char*>(&iBoneIndex), sizeof(uint));

        Matrix4 identityMatrix;
        outfile.write(reinterpret_cast<const char*>(identityMatrix.data()), sizeof(float)*16);

        outfile.write(reinterpret_cast<const char*>(&iNumAffectVertex), sizeof(uint));
        for(uint i = 0; i < iNumAffectVertex; i++)
        {
            vecBoneAffectedVertex[i].ExportToBinary(outfile);
        }
    }
    uint iBoneIndex = {0};
    uint iNumAffectVertex = {0};
    vector<BoneAffectedVertexData> vecBoneAffectedVertex;
};

struct MeshData
{
    void ExportToBinary(ofstream& outfile)
    {
        unsigned int iLength = 0;
        //Material
        outfile.write(reinterpret_cast<const char*>(&iMaterialIndex), sizeof(uint));
        //Texturing
        outfile.write(reinterpret_cast<const char*>(&iTexturingIndex), sizeof(uint));

        //Name
        iLength = strMeshName.length();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        outfile.write(strMeshName.toStdString().c_str(), strMeshName.length());

        outfile.write(reinterpret_cast<const char*>(&iNumVerticies), sizeof(uint));
        outfile.write(reinterpret_cast<const char*>(&iNumFaces), sizeof(uint));

        for(uint i = 0 ; i < iNumVerticies; i++)
        {
            outfile.write(reinterpret_cast<const char*>(vecPosition[i].data()), sizeof(float)*3);
            outfile.write(reinterpret_cast<const char*>(vecNormal[i].data()), sizeof(float)*3);
            outfile.write(reinterpret_cast<const char*>(vecTexCoord[i].data()), sizeof(float)*2);
            outfile.write(reinterpret_cast<const char*>(vecTangent[i].data()), sizeof(float)*3);
        }
        outfile.write(reinterpret_cast<const char*>(&iNumMeshBone), sizeof(unsigned int));
        for(uint i = 0 ; i < iNumMeshBone; i++)
        {
            vecMeshBone[i].ExportToBinary(outfile);
        }
        for(uint i = 0 ; i < iNumFaces*3; )
        {
            outfile.write(reinterpret_cast<const char*>(& vecIndex[i++]), sizeof(unsigned int));
            outfile.write(reinterpret_cast<const char*>(& vecIndex[i++]), sizeof(unsigned int));
            outfile.write(reinterpret_cast<const char*>(& vecIndex[i++]), sizeof(unsigned int));
        }
        outfile.write(reinterpret_cast<const char*>(& iAlpahFalgs), sizeof(unsigned short));
    }

    int iMaterialIndex = {-1};
    QVector<uint> vecMaterialControllerIndex;
    int iTexturingIndex = {-1};
    QVector<uint> vecTextureControllerIndex;

    QString strMeshName = "";
    uint iNumVerticies = {0};
    uint iNumFaces = {0};
    QVector<Vector3> vecPosition;
    QVector<Vector3> vecNormal;
    QVector<Vector2> vecTexCoord;
    QVector<Vector3> vecTangent;
    uint iNumMeshBone = {0};
    QVector<MeshBoneData> vecMeshBone;
    QVector<uint> vecIndex;
    ushort iAlpahFalgs = {0};
};
void ExportMesh( NifModel* nifModel, ofstream& outfile)
{
    list<QModelIndex> MeshList;
    unsigned int iMeshCount = 0;
    int blockCount = nifModel->getBlockCount() ;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if(nifModel->itemName(iBlock) == "NiMesh")
       {
           MeshList.push_back(iBlock);
           iMeshCount++;
       }
    }


    outfile.write(reinterpret_cast<const char*>(&iMeshCount), sizeof( unsigned int));

    vector<MeshData> vecMeshData;
    for(QModelIndex& meshIndex : MeshList)
    {
        MeshData tMeshData;
        //자식 노드 중 Material 인덱스 찾기==========================================================
        auto liChildLinks = nifModel->getChildLinks(nifModel->getBlockNumber(meshIndex));
        for(int iLink : liChildLinks)
        {
            QModelIndex iChildBlockIdx = nifModel->getBlock(iLink);

            if (nifModel->itemName(iChildBlockIdx) == "NiMaterialProperty"  )
            {
                QString materialName = nifModel->get<QString>(iChildBlockIdx,"Name");
                tMeshData.iMaterialIndex = mapMaterialIndex[materialName];

                //Controller
                QModelIndex iCtrl = nifModel->getBlock( nifModel->getLink( iChildBlockIdx, "Controller" ) );
                while ( iCtrl.isValid() && nifModel->inherits( iCtrl, "NiTimeController" ) )
                {
                    if(mapControllerIndex.find(iCtrl) != mapControllerIndex.end())
                    {
                        tMeshData.vecMaterialControllerIndex.push_back( mapControllerIndex[iCtrl]);
                    }
                    iCtrl = nifModel->getBlock( nifModel->getLink( iCtrl, "Next Controller" ) );
                }
            }
            else if (nifModel->itemName(iChildBlockIdx) == "NiTexturingProperty"  )
            {
                tMeshData.iTexturingIndex = mapTexturingIndex[iChildBlockIdx];
                //Controller
                QModelIndex iCtrl = nifModel->getBlock( nifModel->getLink( iChildBlockIdx, "Controller" ) );
                while ( iCtrl.isValid() && nifModel->inherits( iCtrl, "NiTimeController" ) )
                {
                    if(mapControllerIndex.find(iCtrl) != mapControllerIndex.end())
                    {
                        tMeshData.vecTextureControllerIndex.push_back( mapControllerIndex[iCtrl]);

                    }
                    iCtrl = nifModel->getBlock( nifModel->getLink( iCtrl, "Next Controller" ) );
                }
            }
            else if (nifModel->itemName(iChildBlockIdx) == "NiAlphaProperty"  )
            {
               tMeshData.iAlpahFalgs = nifModel->get<int>( iChildBlockIdx, "Flags" );
            }
        }

        //메쉬 정점, 인덱스 데이터 ==================================================================
        QModelIndex iData = nifModel->getIndex( meshIndex, "Datastreams" );
        assert( iData.isValid() );
        int iDataStreamCount = nifModel->rowCount( iData );
        //메쉬 이름
        tMeshData.strMeshName = nifModel->get<QString>(meshIndex,"Name");

        using CompSemIdxMap = QVector<QPair<NiMesh::Semantic, uint>>;
        QVector<CompSemIdxMap> compSemanticIndexMaps;
        for ( int i = 0; i < iDataStreamCount; i++ ) {
            auto stream = nifModel->getLink( iData.child( i, 0 ), "Stream" );
            auto iDataStream = nifModel->getBlock( stream );

            auto usage = NiMesh::DataStreamUsage( nifModel->get<uint>( iDataStream, "Usage" ) );
            auto access = nifModel->get<uint>( iDataStream, "Access" );

            // Invalid Usage and Access, abort
            if ( usage == access && access == 0 )
                return;

            // For each datastream, store the semantic and the index (used for E_TEXCOORD)
            auto iComponentSemantics = nifModel->getIndex( iData.child( i, 0 ), "Component Semantics" );
            uint numComponents = nifModel->get<uint>( iData.child( i, 0 ), "Num Components" );
            CompSemIdxMap compSemanticIndexMap;
            for ( uint j = 0; j < numComponents; j++ ) {
                auto name = nifModel->get<QString>( iComponentSemantics.child( j, 0 ), "Name" );
                auto sem = NiMesh::semanticStrings.value( name );
                uint idx = nifModel->get<uint>( iComponentSemantics.child( j, 0 ), "Index" );
                compSemanticIndexMap.insert( j, {sem, idx} );

                // Create UV stubs for multi-coord systems
            }

            compSemanticIndexMaps << compSemanticIndexMap;
        }

        unique_ptr< NifModel> tempMdl = std::make_unique<NifModel>( nifModel );
        for(int i = 0; i < iDataStreamCount; i++)
        {
           quint32 stream = nifModel->getLink( iData.child( i, 0 ), "Stream" );
           QModelIndex iDataStream = nifModel->getBlock( stream );

           NiMesh::DataStreamUsage usage = NiMesh::DataStreamUsage(nifModel->get<uint>( iDataStream, "Usage" ));


           //정점 갯수
            QVector<QPair<quint32, quint32>> regions;
            quint32 numRegions = nifModel->get<quint32>( iDataStream, "Num Regions" );
            auto iRegions = nifModel->getIndex( iDataStream, "Regions" );
            if ( iRegions.isValid() ) {
                for ( quint32 j = 0; j < numRegions; j++ ) {
                    regions.append( { nifModel->get<quint32>( iRegions.child( j, 0 ), "Start Index" ),
                                    nifModel->get<quint32>( iRegions.child( j, 0 ), "Num Indices" ) }
                    );
                    if(usage ==NiMesh::USAGE_VERTEX )
                        tMeshData.iNumVerticies += regions[j].second;
                    else if(usage ==NiMesh::USAGE_VERTEX_INDEX)
                        tMeshData.iNumFaces += regions[j].second;
                }
            }
           if(usage ==NiMesh::USAGE_VERTEX )
           {
               tMeshData.vecPosition.resize(tMeshData.iNumVerticies);
               tMeshData.vecNormal.resize(tMeshData.iNumVerticies);
               tMeshData.vecTexCoord.resize(tMeshData.iNumVerticies);
               tMeshData.vecTangent.resize(tMeshData.iNumVerticies);
           }
           else if(usage ==NiMesh::USAGE_VERTEX_INDEX)
           {
               tMeshData.vecIndex.reserve(tMeshData.iNumVerticies);
                tMeshData.iNumFaces /=3;
           }

           QVector<NiMesh::DataStreamFormat> datastreamFormats;
           uint numStreamComponents = nifModel->get<uint>( iDataStream, "Num Components" );
           for ( uint j = 0; j < numStreamComponents; j++ ) {
               auto format = nifModel->get<uint>( nifModel->getIndex( iDataStream, "Component Formats" ).child( j, 0 ) );
               datastreamFormats.append( NiMesh::DataStreamFormat(format) );
           }

           QByteArray streamData = nifModel->get<QByteArray>( nifModel->getIndex( iDataStream, "Data" ).child( 0, 0 ) );
           QBuffer streamBuffer( &streamData );
           streamBuffer.open( QIODevice::ReadOnly );
           NifIStream tempInput( tempMdl.get(), &streamBuffer );
           NifValue tempValue;
           int typeLength = ( (NiMesh::F_FLOAT32_3 & typeLengthMask) >> typeLengthShift );
           for ( int l = 0; l < typeLength; l++ ) {
               tempInput.read( tempValue );
           }


           bool abort = false;
           for ( const auto & r : regions ) for ( uint j = 0; j < r.second; j++ ) {
               auto off = r.first;
               Q_ASSERT( tMeshData.iNumFaces*3 >= off + j );
               for ( uint k = 0; k < numStreamComponents; k++ ) {
                   auto typeK = datastreamFormats[k];
                   int typeLength = ( (typeK & typeLengthMask) >> typeLengthShift );
                   switch ( (typeK & typeCodeMask) >> typeCodeShift ) {
                   case 0x10:
                       tempValue.changeType( NifValue::tByte );
                       break;
                   case 0x11:
                       if ( typeK == NiMesh::F_NORMUINT8_4 )
                           tempValue.changeType( NifValue::tByteColor4 );
                       typeLength = 1;
                       break;
                   case 0x13:
                       if ( typeK == NiMesh::F_NORMUINT8_4_BGRA )
                           tempValue.changeType( NifValue::tByteColor4 );
                       typeLength = 1;
                       break;
                   case 0x21:
                       tempValue.changeType( NifValue::tShort );
                       break;
                   case 0x23:
                       if ( typeLength == 3 )
                           tempValue.changeType( NifValue::tHalfVector3 );
                       else if ( typeLength == 2 )
                           tempValue.changeType( NifValue::tHalfVector2 );
                       else if ( typeLength == 1 )
                           tempValue.changeType( NifValue::tHfloat );

                       typeLength = 1;

                       break;
                   case 0x42:
                       tempValue.changeType( NifValue::tInt );
                       break;
                   case 0x43:
                       if ( typeLength == 3 )
                           tempValue.changeType( NifValue::tVector3 );
                       else if ( typeLength == 2 )
                           tempValue.changeType( NifValue::tVector2 );
                       else if ( typeLength == 4 )
                           tempValue.changeType( NifValue::tVector4 );
                       else if ( typeLength == 1 )
                           tempValue.changeType( NifValue::tFloat );

                       typeLength = 1;

                       break;
                   }

                   for ( int l = 0; l < typeLength; l++ ) {
                       tempInput.read( tempValue );
                   }

                   auto compType = compSemanticIndexMaps[i].value( k ).first;
                   switch ( typeK )
                   {
                   case NiMesh::F_FLOAT32_3:
                   case NiMesh::F_FLOAT16_3:
                       Q_ASSERT( usage == NiMesh::USAGE_VERTEX );
                       switch ( compType ) {
                       case NiMesh::E_POSITION:
                       case NiMesh::E_POSITION_BP:
                           tMeshData.vecPosition[j + off] = tempValue.get<Vector3>();
                           break;
                       case NiMesh::E_NORMAL:
                       case NiMesh::E_NORMAL_BP:
                           tMeshData.vecNormal[j + off] = tempValue.get<Vector3>();
                           break;
                       case NiMesh::E_TANGENT:
                       case NiMesh::E_TANGENT_BP:
                           tMeshData.vecTangent[j + off] = tempValue.get<Vector3>();
                           break;
                       case NiMesh::E_BINORMAL:
                       case NiMesh::E_BINORMAL_BP:
                           break;
                       default:
                           break;
                       }
                       break;
                   case NiMesh::F_UINT16_1:
                       if ( compType == NiMesh::E_INDEX ) {
                           Q_ASSERT( usage == NiMesh::USAGE_VERTEX_INDEX );
                           // TODO: The total index value across all submeshes
                           // is likely allowed to exceed USHRT_MAX.
                           // For now limit the index.
                           quint32 ind = tempValue.get<quint16>() + off;
                           ind = std::min( ind, indexLimit );

                           tMeshData.vecIndex.append( ind );
                       }
                       break;
                   case NiMesh::F_FLOAT32_2:
                   case NiMesh::F_FLOAT16_2:
                       Q_ASSERT( usage == NiMesh::USAGE_VERTEX );
                       if ( compType == NiMesh::E_TEXCOORD ) {
                           quint32 coordSet = compSemanticIndexMaps[i].value( k ).second;
                           Q_ASSERT( tMeshData.vecTexCoord.size() > coordSet );
                           tMeshData.vecTexCoord[j + off] = tempValue.get<Vector2>();
                       }
                       break;
                   case NiMesh::F_UINT8_4:
                       // BLENDINDICES, do nothing for now
                       break;
                   case NiMesh::F_NORMUINT8_4:
                       Q_ASSERT( usage == NiMesh::USAGE_VERTEX );
                       if ( compType == NiMesh::E_COLOR )
                       break;
                   case NiMesh::F_NORMUINT8_4_BGRA:
                       Q_ASSERT( usage == NiMesh::USAGE_VERTEX );
                       break;
                   default:
                       abort = true;
                       break;
                   }

                   if ( abort == true )
                       break;
               }
           }

           // Clear is extremely expensive. Must be outside of loop
           tempMdl->clear();

        }
        //메쉬 본 데이터 =================================================================
        tMeshData.iNumMeshBone = 1;
        MeshBoneData tMeshBoneData;
        tMeshBoneData.iBoneIndex = mapBoneIndex[meshIndex];
        tMeshBoneData.iNumAffectVertex = tMeshData.iNumVerticies;
        tMeshData.vecMeshBone.push_back(tMeshBoneData);
        tMeshData.vecMeshBone[0].vecBoneAffectedVertex.resize(tMeshBoneData.iNumAffectVertex);

        for(int iVtx = 0 ; iVtx <tMeshData.iNumVerticies; iVtx++ )
        {
           tMeshData.vecMeshBone[0].vecBoneAffectedVertex[iVtx].iVertexIndex = iVtx;
           tMeshData.vecMeshBone[0].vecBoneAffectedVertex[iVtx].fWeight = 1;
        }


        vecMeshData.push_back(tMeshData);
    }
    for(MeshData& mesh : vecMeshData)
    {
        mesh.ExportToBinary(outfile);
    }
}
//============================================ MATERIAL================================================
struct MaterialData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        if (false == index.isValid())
            return;

        fAlpha = nif->get<float>( index, "Alpha" );
        if ( fAlpha < 0.0 )
            fAlpha = 0.0;

        if ( fAlpha > 1.0 )
            fAlpha = 1.0;

        fAmbient  = (nif->get<Color3>( index, "Ambient Color" ) );
        fDiffuse  = (nif->get<Color3>( index, "Diffuse Color" ) );
        fSpecular = (nif->get<Color3>( index, "Specular Color" )) ;
        fEmmisive = (nif->get<Color3>( index, "Emissive Color" )) ;

        // OpenGL needs shininess clamped otherwise it generates GL_INVALID_VALUE
        fGlossiness = std::min( std::max( nif->get<float>( index, "Glossiness" ), 0.0f ), maxGlossiness );
    }

    void ExportToBinary(ofstream& outfile)
    {
        outfile.write(reinterpret_cast<const char*>( fAmbient.data()), sizeof(float)*3);
        outfile.write(reinterpret_cast<const char*>( fDiffuse.data()), sizeof(float)*3);
        outfile.write(reinterpret_cast<const char*>( fSpecular.data()), sizeof(float)*3);
        outfile.write(reinterpret_cast<const char*>( fEmmisive.data()), sizeof(float)*3);
        outfile.write(reinterpret_cast<const char*>(& fGlossiness), sizeof(float));
        outfile.write(reinterpret_cast<const char*>(& fAlpha), sizeof(float));
    }

    Color3 fAmbient = {0,0,0};
    Color3 fDiffuse = {0,0,0};
    Color3 fSpecular = {0,0,0};
    Color3 fEmmisive = {0,0,0};
    float fGlossiness = {0};
    float fAlpha = {0};
};


void ExportMaterial( class NifModel* nifModel,std::ofstream& outfile)
{
    uint iLength = mapMaterialIndex.size();
    outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
    int blockCount = nifModel->getBlockCount() ;

    map<QString, QModelIndex> mapMaterialBlock;
    for (int i = 0; i < blockCount; ++i) {
       QModelIndex iBlock = nifModel->getBlock(i);
       if (nifModel->inherits( iBlock, "NiMaterialProperty" ))
       {
           QString strCurrentMaterialName = nifModel->get<QString>(iBlock,"Name");
           if(mapMaterialBlock.find(strCurrentMaterialName) == mapMaterialBlock.end())
               mapMaterialBlock[strCurrentMaterialName] = iBlock;
       }
    }

    vector<pair<QString,unsigned int>> vecMaterialIdx(mapMaterialIndex.begin(), mapMaterialIndex.end());
    std::sort(vecMaterialIdx.begin(), vecMaterialIdx.end(), [](const auto& a, const auto& b) {
           return a.second < b.second; // 사전순 정렬
       });
    for(auto& material : vecMaterialIdx)
    {
        auto iter = mapMaterialBlock.find(material.first);
        if(iter == mapMaterialBlock.end())
            continue;

        MaterialData tMatData;
        tMatData.update(nifModel,iter->second);
        tMatData.ExportToBinary(outfile);
    }

}
//============================================ TEXTURE================================================
struct TextureData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        strTextureFileName = nif->get<QString>( index, "File Name" );
    }
    void ExportToBinary(ofstream& outfile)
    {
        uint iNumSRV = 1;
        outfile.write(reinterpret_cast<const char*>(& iNumSRV), sizeof(uint));
        uint iLength = strTextureFileName.size();
        outfile.write(reinterpret_cast<const char*>(& iLength), sizeof(uint));
        outfile.write(strTextureFileName.toStdString().c_str(), iLength);
    }
    QString  strTextureFileName;
};
void ExportTexture(NifModel* nifModel, ofstream& outfile)
{
    uint iLength = mapTextureIndex.size();
    outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));

    for(auto& texture : mapTextureIndex)
    {
        TextureData  tTextureData;
        tTextureData.update(nifModel,texture.first);
        tTextureData.ExportToBinary(outfile);
    }

}
struct TextureSlotData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        if (false == index.isValid())
            return;

        QModelIndex qiSourceTexIndex = nif->getBlock( nif->getLink( index, "Source" ), "NiSourceTexture" );
        iTextureIndex = mapTextureIndex[qiSourceTexIndex];
        bHasTextureTransform = nif->get<int>( index, "Has Texture Transform" );

        if ( bHasTextureTransform ) {
            vTranslation = nif->get<Vector2>( index, "Translation" );
            vScale = nif->get<Vector2>( index, "Scale" );
            fRotation = nif->get<float>( index, "Rotation" );
            vCenter = nif->get<Vector2>( index, "Center" );
        }
    }

    void ExportToBinary(ofstream& outfile)
    {
        outfile.write(reinterpret_cast<const char*>(& eTextureType), sizeof(uint));


        outfile.write(reinterpret_cast<const char*>(& iTextureIndex), sizeof(uint));
        outfile.write(reinterpret_cast<const char*>(& bHasTextureTransform), sizeof(bool));
        outfile.write(reinterpret_cast<const char*>( vTranslation.data()), sizeof(float)*2);
        outfile.write(reinterpret_cast<const char*>( vScale.data()), sizeof(float)*2);
        outfile.write(reinterpret_cast<const char*>(& fRotation), sizeof(float));
        outfile.write(reinterpret_cast<const char*>( vCenter.data()), sizeof(float)*2);
    }

    TEX_TYPE eTextureType;
    uint iTextureIndex;
    bool bHasTextureTransform;
    Vector2 vTranslation = {0,0};
    Vector2 vScale = {1,1};
    float fRotation = {0};
    Vector2 vCenter = {0.5,0.5};
};


struct TexturingData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        if (false == index.isValid())
            return;

        vecTexture.clear();
        for ( uint t = 0; t < TT_LAST; t++ )
        {
            QModelIndex iTex = nif->getIndex( index, texnames[t] );

            if (false == iTex.isValid() ) continue;

            TextureSlotData tTextureData;
            tTextureData.eTextureType = (TEX_TYPE)t;
            tTextureData.update(nif, iTex);
            vecTexture.push_back(tTextureData);
        }

    }

    void ExportToBinary(ofstream& outfile)
    {
        uint iLength = vecTexture.size();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        for(TextureSlotData& tex : vecTexture)
            tex.ExportToBinary(outfile);
    }

    QVector<TextureSlotData > vecTexture;
};


void ExportTexturing( class NifModel* nifModel,std::ofstream& outfile)
{
    uint iLength = mapTexturingIndex.size();
    outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
    for(auto& texturing : mapTexturingIndex)
    {
        TexturingData tTexturingData;
        tTexturingData.update(nifModel,texturing.first);
        tTexturingData.ExportToBinary(outfile);
    }
}
//============================================ CONTROLLER================================================
struct ControllerData
{
    virtual ~ControllerData() = default;
    virtual void update( const NifModel * nif, const QModelIndex & index )
    {
        if(false == index.isValid())
            return;

        QModelIndex iNextCtrl = nif->getBlock( nif->getLink( index, "Next Controller" ) );
        if(iNextCtrl.isValid())
            if(mapControllerIndex.find(iNextCtrl) != mapControllerIndex.end())
                iNextControllerIndex = mapControllerIndex[iNextCtrl];

        fStartTime = nif->get<float>( index, "Start Time" );
        fStopTime  = nif->get<float>( index, "Stop Time" );
        fFrequency = nif->get<float>( index, "Frequency" );
        fPhase = nif->get<float>( index, "Phase" );
        qiTarget = nif->getBlock( nif->getLink( index, "Target" ) );

        qiInterpoloatr = nif->getBlock( nif->getLink( index, "Interpolator" ) );
        if(qiInterpoloatr.isValid())
            qiData = nif->getBlock( nif->getLink( qiInterpoloatr, "Data" ) );
        sFlags = nif->get<int>( index, "Flags" );
    }
    virtual void ExportToBinary(ofstream& outfile)
    {
        outfile.write(reinterpret_cast<const char*>(&eType), sizeof(uint));
        outfile.write(reinterpret_cast<const char*>(&fStartTime), sizeof(float));
        outfile.write(reinterpret_cast<const char*>(&fStopTime), sizeof(float));
        outfile.write(reinterpret_cast<const char*>(&fFrequency), sizeof(float));
        outfile.write(reinterpret_cast<const char*>(&fPhase), sizeof(float));
        outfile.write(reinterpret_cast<const char*>(&iTargetIndex), sizeof(int));
        outfile.write(reinterpret_cast<const char*>(&sFlags), sizeof(unsigned short));
    }
    QModelIndex qiInterpoloatr;
    QModelIndex qiData;
    QModelIndex qiTarget;

    CTRL_TYPE eType = {CT_LAST};
    int iNextControllerIndex = {-1};
    float fStartTime = {0};
    float fStopTime = {0};
    float fFrequency = {1};
    float fPhase = {0};
    int iTargetIndex = {-1};
    unsigned short sFlags = {0};
};

typedef struct TransformKeyframe
{
    TransformKeyframe()
    {
        vScale = {1,1,1};
        vRotation = {0,0,0};
        vPosition = {0,0,0};
        fTrackPosition = {0};
    }

    Vector3     vScale = {1,1,1};
    Vector3     vRotation = {0,0,0};
    Vector3     vPosition = {0,0,0};
    float       fTrackPosition = {0};
}TRANSFORM_KEYFRAME;
typedef struct FloatKeyframe
{
    float       fValue = {0};
    float       fTrackPosition = {0};
}FLOAT_KEYFRAME;
struct TransformControllerData : public ControllerData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        eType = CT_TRANSFORM;
        ControllerData::update(nif,index);
        iTargetIndex = mapBoneIndex[qiTarget];
        //크기
        QModelIndex qiScale = nif->getIndex( qiData, "Scales" );
        const NifModel * nifScale = static_cast<const NifModel *>( qiScale.model() );
        if (qiScale.isValid() ) {
            QModelIndex qiFrames = nifScale->getIndex( qiScale, "Keys" );
            uint iNumKeys = nifScale->get<uint>(qiScale, "Num Keys");
            for(uint iKeyIndex = 0 ; iKeyIndex < iNumKeys; iKeyIndex++ )
            {
                float fTime = nifScale->get<float>( qiFrames.child( iKeyIndex, 0 ), "Time" );
                float fValue = nifScale->get<float>( qiFrames.child( iKeyIndex, 0 ), "Value" );
                auto itPrev = mapKeyFrame.lower_bound(fTime);

                if (itPrev != mapKeyFrame.end() && itPrev->first == fTime) {
                    // 동일한 시간이 존재하면 갱신
                    itPrev->second.vScale= {fValue,fValue,fValue};
                } else {
                    // 새 키프레임 추가
                    TRANSFORM_KEYFRAME tKey;
                    if(itPrev != mapKeyFrame.begin())
                    {
                        tKey = (--itPrev)->second;
                    }
                    tKey.fTrackPosition = fTime;
                    tKey.vScale = {fValue,fValue,fValue};
                    mapKeyFrame[fTime] = tKey;
                }
            }
        }
        //회전
        QModelIndex qiXYZRotation = nif->getIndex( qiData, "XYZ Rotations" );
        if ( qiXYZRotation.isValid() ) {

            for ( int s = 0; s < 3 && s < nif->rowCount( qiXYZRotation ); s++ )
            {
                QModelIndex qiAxis = qiXYZRotation.child( s, 0 );
                const NifModel * nifAxis = static_cast<const NifModel *>( qiAxis.model() );

                if ( nifAxis && qiAxis.isValid() ) {
                    QModelIndex qiFrames = nifAxis->getIndex( qiAxis, "Keys" );
                    if(false == qiFrames.isValid())
                        continue;

                    uint iNumKeys = nifAxis->get<uint>(qiAxis, "Num Keys");
                    for(uint iKeyIndex = 0 ; iKeyIndex < iNumKeys; iKeyIndex++ )
                    {
                        float fTime = nifAxis->get<float>( qiFrames.child( iKeyIndex, 0 ), "Time" );
                        float fValue = nifAxis->get<float>( qiFrames.child( iKeyIndex, 0 ), "Value" );

                        auto itPrev = mapKeyFrame.lower_bound(fTime);

                        if (itPrev != mapKeyFrame.end() && itPrev->first == fTime) {
                            // 동일한 시간이 존재하면 갱신
                            itPrev->second.vRotation[s] = fValue;
                        } else {
                            // 새 키프레임 추가
                            TRANSFORM_KEYFRAME tKey;
                            if(itPrev != mapKeyFrame.begin())
                            {
                                tKey = (--itPrev)->second;
                            }
                            tKey.fTrackPosition = fTime;
                            tKey.vRotation[s] = fValue;
                            mapKeyFrame[fTime] = tKey;
                        }
                    }
                }
            }


        }
        //위치
        QModelIndex qiPosition = nif->getIndex( qiData, "Translations" );
        const NifModel * nifTranslation = static_cast<const NifModel *>( qiPosition.model() );
        if (qiPosition.isValid() ) {
            QModelIndex qiFrames = nifTranslation->getIndex( qiPosition, "Keys" );
            uint iNumKeys = nifTranslation->get<uint>(qiPosition, "Num Keys");
            for(uint iKeyIndex = 0 ; iKeyIndex < iNumKeys; iKeyIndex++ )
            {
                float fTime = nifTranslation->get<float>( qiFrames.child( iKeyIndex, 0 ), "Time" );
                Vector3 vValue = nifTranslation->get<Vector3>( qiFrames.child( iKeyIndex, 0 ), "Value" );


                auto itPrev = mapKeyFrame.lower_bound(fTime);

                if (itPrev != mapKeyFrame.end() && itPrev->first == fTime) {
                    // 동일한 시간이 존재하면 갱신
                    itPrev->second.vPosition = vValue;
                } else {
                    // 새 키프레임 추가
                    TRANSFORM_KEYFRAME tKey;
                    if(itPrev != mapKeyFrame.begin())
                    {
                        tKey = (--itPrev)->second;
                    }
                    tKey.fTrackPosition = fTime;
                    tKey.vPosition = vValue;
                    mapKeyFrame[fTime] = tKey;
                }
            }
        }

    }

    void ExportToBinary(ofstream& outfile)
    {
        ControllerData::ExportToBinary(outfile);
        uint iLength = mapKeyFrame.size();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        for(auto& keyFramePair : mapKeyFrame)
        {
            TRANSFORM_KEYFRAME keyFrame = keyFramePair.second;
            outfile.write(reinterpret_cast<const char*>(keyFrame.vScale.data()), sizeof(float)*3);
            outfile.write(reinterpret_cast<const char*>(keyFrame.vRotation.data()), sizeof(float)*3);
            outfile.write(reinterpret_cast<const char*>(keyFrame.vPosition.data()), sizeof(float)*3);
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fTrackPosition), sizeof(float));
        }
        outfile.write(reinterpret_cast<const char*>(&sFlags), sizeof(unsigned short));

    }
    map<float, TRANSFORM_KEYFRAME> mapKeyFrame;

};
struct AlphaControllerData : public ControllerData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        eType = CT_ALPHA;
        ControllerData::update(nif,index);
        if (false == qiData.isValid() || false == nif->inherits(qiData, "NiFloatData"))
               return;
         QString materialName = nif->get<QString>(qiTarget ,"Name");
         iTargetIndex = mapMaterialIndex[materialName];

        QModelIndex array = nif->getIndex( qiData, "Data" );
        const NifModel * nifModel = static_cast<const NifModel *>( array.model() );
        if ( nifModel && array.isValid() )
        {
            QModelIndex qiFrames = nifModel->getIndex( array, "Keys" );
            if(false == qiFrames.isValid())
                return;
            uint iNumKeys = nifModel->get<uint>( array, "Num Keys" );
            for(uint iKeyIndex = 0 ; iKeyIndex < iNumKeys; iKeyIndex++ )
            {
                float fTime = nif->get<float>( qiFrames.child(iKeyIndex,0), "Time" );
                float fValue = nif->get<float>( qiFrames.child(iKeyIndex,0), "Value" );

                auto itPrev = mapKeyFrame.lower_bound(fTime);

                if (itPrev != mapKeyFrame.end() && itPrev->first == fTime) {
                    // 동일한 시간이 존재하면 갱신
                    itPrev->second.fValue = fValue;
                } else {
                    // 새 키프레임 추가
                    FLOAT_KEYFRAME tKey = (itPrev == mapKeyFrame.begin()) ? FLOAT_KEYFRAME() : (--itPrev)->second;
                    tKey.fTrackPosition = fTime;
                    tKey.fValue = fValue;
                    mapKeyFrame[fTime] = tKey;
                }
            }

        }


    }

    void ExportToBinary(ofstream& outfile)
    {
        ControllerData::ExportToBinary(outfile);
        uint iLength = mapKeyFrame.size();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        for(auto& keyFramePair : mapKeyFrame)
        {
            FLOAT_KEYFRAME keyFrame = keyFramePair.second;
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fValue), sizeof(float));
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fTrackPosition), sizeof(float));
        }
    }
    map<float,FLOAT_KEYFRAME> mapKeyFrame;
};
struct TextureTransformControllerData : public ControllerData
{

    void update( const NifModel * nif, const QModelIndex & index )
    {
        eType = CT_TEXTURE_TRANSFORM;
        ControllerData::update(nif,index);
        eTexSlot = uint(nif->get<int>( index, "Texture Slot" ));
        eTexOperation = uint(nif->get<int>( index, "Operation" ));
        iTargetIndex = mapTexturingIndex[qiTarget];

        QModelIndex array = nif->getIndex( qiData, "Data" );
        const NifModel * nifModel = static_cast<const NifModel *>( array.model() );
        if ( nifModel && array.isValid() )
        {
            QModelIndex qiFrames = nifModel->getIndex( array, "Keys" );
            if(false == qiFrames.isValid())
                return;
            uint iNumKeys = nifModel->get<uint>( array, "Num Keys" );
            for(uint iKeyIndex = 0 ; iKeyIndex < iNumKeys; iKeyIndex++ )
            {
                float fTime = nif->get<float>( qiFrames.child(iKeyIndex,0), "Time" );
                float fValue = nif->get<float>( qiFrames.child(iKeyIndex,0), "Value" );

                auto itPrev = mapKeyFrame.lower_bound(fTime);

                if (itPrev != mapKeyFrame.end() && itPrev->first == fTime) {
                    // 동일한 시간이 존재하면 갱신
                    itPrev->second.fValue = fValue;
                } else {
                    // 새 키프레임 추가
                    FLOAT_KEYFRAME tKey = (itPrev == mapKeyFrame.begin()) ? FLOAT_KEYFRAME() : (--itPrev)->second;
                    tKey.fTrackPosition = fTime;
                    tKey.fValue = fValue;
                    mapKeyFrame[fTime] = tKey;
                }
            }

        }

    }

    void ExportToBinary(ofstream& outfile)
    {
        ControllerData::ExportToBinary(outfile);
        outfile.write(reinterpret_cast<const char*>(&eTexSlot), sizeof(uint));
        outfile.write(reinterpret_cast<const char*>(&eTexOperation), sizeof(uint));

        uint iLength = mapKeyFrame.size();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        for(auto& keyFramePair : mapKeyFrame)
        {
            FLOAT_KEYFRAME keyFrame = keyFramePair.second;
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fValue), sizeof(float));
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fTrackPosition), sizeof(float));
        }
    }
    uint eTexSlot = TEX_TYPE::TT_LAST;
    uint eTexOperation = TEX_OPERATION_TYPE::TO_LAST;
    map<float, FLOAT_KEYFRAME> mapKeyFrame;
};
struct FlipControllerData : public ControllerData
{

    void update( const NifModel * nif, const QModelIndex & index )
    {
        eType = CT_TEXTURE_FLIP;
        ControllerData::update(nif,index);
        eTexSlot = uint(nif->get<int>( index, "Texture Slot" ));
        iTargetIndex = mapTexturingIndex[qiTarget];

        QModelIndex array = nif->getIndex( qiData, "Data" );
        const NifModel * nifModel = static_cast<const NifModel *>( array.model() );
        if ( nifModel && array.isValid() )
        {
            QModelIndex qiFrames = nifModel->getIndex( array, "Keys" );
            if(false == qiFrames.isValid())
                return;
            uint iNumKeys = nifModel->get<uint>( array, "Num Keys" );
            for(uint iKeyIndex = 0 ; iKeyIndex < iNumKeys; iKeyIndex++ )
            {
                float fTime = nif->get<float>( qiFrames.child(iKeyIndex,0), "Time" );
                float fValue = nif->get<float>( qiFrames.child(iKeyIndex,0), "Value" );

                auto itPrev = mapKeyFrame.lower_bound(fTime);

                if (itPrev != mapKeyFrame.end() && itPrev->first == fTime) {
                    // 동일한 시간이 존재하면 갱신
                    itPrev->second.fValue = fValue;
                } else {
                    // 새 키프레임 추가
                    FLOAT_KEYFRAME tKey = (itPrev == mapKeyFrame.begin()) ? FLOAT_KEYFRAME() : (--itPrev)->second;
                    tKey.fTrackPosition = fTime;
                    tKey.fValue = fValue;
                    mapKeyFrame[fTime] = tKey;
                }
            }

        }


        iSourceCount = nif->get<uint> (index, "Num Sources");
        QModelIndex qiSources = nif->getIndex( index, "Sources" );
        if(qiSources.isValid() == false)
            return;
        const NifModel * nifSources = static_cast<const NifModel *>(qiSources.model());

        for ( int i = 0; i <iSourceCount; i++ )
        {
            int iSourceTexLink = nifSources->getLink( qiSources.child( (int)i, 0 ));
            QModelIndex qiSourceTexture = nifSources->getBlock(  iSourceTexLink, "NiSourceTexture" );
            if(qiSourceTexture.isValid())
                vecSourceTextures.push_back(mapTextureIndex[qiSourceTexture] ) ;
        }
    }
    void ExportToBinary(ofstream& outfile)
    {
        ControllerData::ExportToBinary(outfile);
        outfile.write(reinterpret_cast<const char*>(&eTexSlot), sizeof(uint));

        uint iLength = mapKeyFrame.size();
        outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
        for(auto& keyFramePair : mapKeyFrame)
        {
            FLOAT_KEYFRAME keyFrame = keyFramePair.second;
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fValue), sizeof(float));
            outfile.write(reinterpret_cast<const char*>(&keyFrame.fTrackPosition), sizeof(float));
        }

        outfile.write(reinterpret_cast<const char*>(&iSourceCount), sizeof(uint));
        for(uint& i : vecSourceTextures)
        {
            outfile.write(reinterpret_cast<const char*>(&i), sizeof(uint));
        }

    }
    uint eTexSlot = TEX_TYPE::TT_LAST;

    map<float, FLOAT_KEYFRAME> mapKeyFrame;
    uint iSourceCount = 0;
    vector<uint> vecSourceTextures;
};

struct MaterialColorControllerData : public ControllerData
{
    void update( const NifModel * nif, const QModelIndex & index )
    {
        eType = CT_MATERIAL_COLOR;
        ControllerData::update(nif,index);
        QString materialName = nif->get<QString>(qiTarget ,"Name");

        iTargetIndex = mapMaterialIndex[materialName];
    }
    void ExportToBinary(ofstream& outfile)
    {
        ControllerData::ExportToBinary(outfile);
    }

};

void ExportController( class NifModel* nifModel,std::ofstream& outfile)
{
    uint iLength = mapControllerIndex.size();
    outfile.write(reinterpret_cast<const char*>(&iLength), sizeof(uint));
    for(auto& controller : mapControllerIndex)
    {
        QModelIndex index = controller.first;
        ControllerData* pControllerData = nullptr;
        if(nifModel->itemName(index) == "NiTransformController")
            pControllerData = new TransformControllerData;
        else if(nifModel->itemName(index) == "NiTextureTransformController")
            pControllerData = new TextureTransformControllerData;
        else if(nifModel->itemName(index) == "NiAlphaController")
            pControllerData = new AlphaControllerData;
        else if(nifModel->itemName(index) == "NiMaterialColorController")
            pControllerData = new MaterialColorControllerData;
        else if(nifModel->itemName(index) == "NiFlipController")
            pControllerData = new FlipControllerData;
        else
            continue;

        pControllerData->update(nifModel,controller.first);
        pControllerData->ExportToBinary(outfile);
        delete pControllerData;
    }
}

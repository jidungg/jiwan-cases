#ifndef NIFTOBINARY_H
#define NIFTOBINARY_H

#include "fstream"


void ExportBone( class NifModel* nifModel,std::ofstream& outfile);
void ExportMesh( class NifModel* nifModel,std::ofstream& outfile);
void ExportMaterial( class NifModel* nifModel,std::ofstream& outfile);
void ExportTexture( class NifModel* nifModel,std::ofstream& outfile);
void ExportTexturing( class NifModel* nifModel,std::ofstream& outfile);
void ExportController( class NifModel* nifModel,std::ofstream& outfile);


void MakeBoneList( NifModel* nifModel, int iIndex = -1);
void MakeMeshList( NifModel* nifModel);
void MakeMaterialList( NifModel* nifModel);
void MakeTextureList( NifModel* nifModel);
void MakeTexturingList( NifModel* nifModel);
void MakeControllerList( NifModel* nifModel);

#endif // NIFTOBINARY_H

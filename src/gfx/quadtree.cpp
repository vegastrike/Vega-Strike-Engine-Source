#include "quadtree.h"
#include "matrix.h"
#include "aux_texture.h"
#include "universe.h"
#include "vegastrike.h"
const GFXVertex InitialVertices [4]= { GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0) };
 
QuadTree::QuadTree (const char * filename, const Vector &Scales, const float radius):minX(0), minZ(0), maxX(0),  maxZ(0),Scales (Scales), vertices (GFXTRI,4,InitialVertices,4,true) {
  detail =128;
  Identity (transformation);
  transformation[0]=Scales.i;
  transformation[5]=Scales.j;
  transformation[10]=Scales.k;
  //  nonlinear_transform = new IdentityTransform;
  RootCornerData.Parent = NULL;
  RootCornerData.Square = NULL;
  RootCornerData.ChildIndex = 0;
  RootCornerData.Level =15;
  RootCornerData.xorg = 0;
  RootCornerData.zorg = 0;
  RootCornerData.Verts[0].Y = 0;   RootCornerData.Verts[0].vertindex=0;
  RootCornerData.Verts[1].Y = 0;   RootCornerData.Verts[1].vertindex=1;
  RootCornerData.Verts[2].Y = 0;   RootCornerData.Verts[2].vertindex=2;
  RootCornerData.Verts[3].Y = 0;   RootCornerData.Verts[3].vertindex=3;
  RootCornerData.Verts[0].SetTex(0);
  RootCornerData.Verts[1].SetTex(0);
  RootCornerData.Verts[2].SetTex(9);
  RootCornerData.Verts[3].SetTex(9);
  

  VertexAllocated = VertexCount = 4;
  /*
  textures.push_back (TerrainTexture());
  for (int i=1;i<10;i++) {
    char name[]="terrainX.bmp";
    name[7] = '0'+i;
    TerrainTexture tmp;
    tmp.tex.t = new Texture (name);
    textures.push_back (tmp);
  }
  */



  if (filename) {
    LoadXML(filename, Scales,radius);
    //adData();
   } else {
     quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform, &textures,Vector (1.0F/Scales.i,1.0F/Scales.j,1.0F/Scales.k));
     root = new quadsquare (&RootCornerData);
  }
  root->StaticCullData (RootCornerData,25);
  /*
        for (i = 0; i < 10; i++) {
                root->Update(RootCornerData, (const float*) ViewerLoc, Detail);
        }
  */

}



QuadTree::~QuadTree () {
  delete root;
  delete nonlinear_transform;
  
}

float QuadTree::GetHeight (Vector Location, Vector & normal) {
  Location = nonlinear_transform->InvTransform (InvScaleTransform (transformation,Location));
  float tmp =  Location.j-root->GetHeight (RootCornerData,Location.i,Location.k,  normal);
  normal = Transform (transformation,nonlinear_transform->TransformNormal (Location, normal));
  normal.Normalize();
  return tmp;
}


void QuadTree::Update (unsigned short numstages, unsigned short whichstage) {
  //GetViewerPosition
  quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform, &textures,Vector (1.0F/Scales.i,1.0F/Scales.j,1.0F/Scales.k));
  root->Update (RootCornerData,nonlinear_transform->InvTransform (InvScaleTransform (transformation,_Universe->AccessCamera()->GetPosition())),detail,numstages,whichstage);
}

inline Vector calculatenormscale (const Matrix trans) {
  return Vector (1.0F/(Vector (trans[0],trans[1],trans[2]).Magnitude()),1.0F/(Vector (trans[4],trans[5],trans[6]).Magnitude()),1.0F/(Vector (trans[8],trans[9],trans[10]).Magnitude()));
}

void QuadTree::SetTransformation(const Matrix mat) {
  memcpy (transformation,mat,sizeof(float)*16);
  Scales= (calculatenormscale (mat));
}


void QuadTree::Render () {
  GFXLoadMatrix (MODEL,transformation);
  GFXBoxInFrustumModel (transformation);
  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXEnable (LIGHTING);
  GFXBlendMode (ONE,ZERO);
  quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform,&textures, Scales);
  root->Render (RootCornerData,nonlinear_transform->InvTransform (InvScaleTransform (transformation,_Universe->AccessCamera()->GetPosition())));
}
void	QuadTree::LoadData()
// Load some data and put it into the quadtree.
{
  nonlinear_transform= new IdentityTransform();
	HeightMapInfo	hm;
	hm.XOrigin = 0;
	hm.ZOrigin = 0;
	hm.XSize = 512;
	hm.ZSize = 512;
	hm.RowWidth = hm.XSize;
	hm.Scale = 7;
	hm.Data = new short[hm.XSize * hm.ZSize];
	hm.terrainmap = new unsigned char  [hm.XSize*hm.ZSize];
	memset (hm.terrainmap,0,sizeof (unsigned char)*hm.ZSize * hm.XSize);
	printf("Loading height grids...\n");

	// Big coarse data, at 128 meter sample spacing.
	FILE*	fp = fopen("demdata/gc16at128.raw", "rb");
	fread(hm.Data, sizeof(unsigned short), hm.XSize * hm.ZSize, fp);
	fclose(fp);
	printf("Building quadtree data...\n");
	root->AddHeightMap(RootCornerData, hm);
	
	// More detailed data at 64 meter spacing, covering the middle of the terrain.
	hm.XOrigin = 16384;
	hm.ZOrigin = 16384;
	hm.Scale = 6;
	fp = fopen("demdata/gc16at64.raw", "rb");
	fread(hm.Data, sizeof(unsigned short), hm.XSize * hm.ZSize, fp);
	fclose(fp);
	printf("Adding quadtree data...\n");
	root->AddHeightMap(RootCornerData, hm);
	
	// Even more detailed data, at 32 meter spacing, covering a smaller area near the middle.
	hm.XOrigin = 24576;
	hm.ZOrigin = 24576;
	hm.Scale = 5;
	fp = fopen("demdata/gc16at32.raw", "rb");
	fread(hm.Data, sizeof(unsigned short), hm.XSize * hm.ZSize, fp);
	fclose(fp);
	printf("Adding quadtree data...\n");
	root->AddHeightMap(RootCornerData, hm);
	
	delete [] hm.Data;
}


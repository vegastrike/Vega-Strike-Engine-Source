#include "quadtree.h"
#include "matrix.h"
#include "aux_texture.h"
#include "universe.h"
#include "vegastrike.h"
const GFXVertex InitialVertices [4]= { GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0) };
 
QuadTree::QuadTree ():vertices (GFXTRI,4,InitialVertices,4,true) {
  Identity (transformation);
  nonlinear_transform = new IdentityTransform;
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
  for (int i=0;i<10;i++) {
    char name[]="terrainX.bmp";
    name[7] = '0'+i;
    textures.push_back (new Texture (name));
  }
  quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform, &textures);
  root = new quadsquare (&RootCornerData);
  LoadData();
}



QuadTree::~QuadTree () {
  delete root;
  delete nonlinear_transform;
  
}
float QuadTree::GetHeight (Vector Location) {
  InvTransform (transformation,Location);
  nonlinear_transform->InvTransform (Location);
  return Location.j-root->GetHeight (RootCornerData,Location.i,Location.k);
}
void QuadTree::Update () {
  //GetViewerPosition
  root->Update (RootCornerData,InvTransform (transformation,_Universe->AccessCamera()->GetPosition()),140);
}

void QuadTree::Render () {
  GFXLoadMatrix (MODEL,transformation);
  GFXBoxInFrustumModel (transformation);
  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXEnable (LIGHTING);
  GFXBlendMode (ONE,ZERO);
  quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform,&textures);
  root->Render (RootCornerData);
}
void	QuadTree::LoadData()
// Load some data and put it into the quadtree.
{
	HeightMapInfo	hm;
	hm.XOrigin = 0;
	hm.ZOrigin = 0;
	hm.XSize = 512;
	hm.ZSize = 512;
	hm.RowWidth = hm.XSize;
	hm.Scale = 7;
	hm.Data = new short[hm.XSize * hm.ZSize];

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


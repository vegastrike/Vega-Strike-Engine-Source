#include "quadtree.h"
#include "matrix.h"
#include "aux_texture.h"
#include "universe.h"
#include "vegastrike.h"
const GFXVertex InitialVertices [4]= { GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0),
				       GFXVertex (Vector(0,0,0),Vector (0,1,0), 0,0) };
 
QuadTree::QuadTree (const char * filename, const Vector &Scales):vertices (GFXTRI,4,InitialVertices,4,true) {
  detail =128;
  Identity (transformation);
  transformation[0]=Scales.i;
  transformation[5]=Scales.j;
  transformation[10]=Scales.k;
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
  quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform, &textures,Vector (1.0F/Scales.i,1.0F/Scales.j,1.0F/Scales.k));


  if (filename) {
    LoadXML(filename);
    //adData();
   } else {
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
///possibly flawed
static Vector InvScaleTransform (Matrix trans,  Vector pos) {
  pos = pos - Vector (trans[12],trans[13],trans[14]);
#define a (trans[0])
#define b (trans[4])
#define c (trans[8])
#define d (trans[1])
#define e (trans[5])
#define f (trans[9])
#define g (trans[2])
#define h (trans[6])
#define i (trans[10])
  float factor = 1.0F/(-c*e*g+ b*f*g + c*d*h - a*f*h - b*d*i + a*e*i);
  return (Vector(pos.Dot (Vector (e*i- f*h,c*h-b*i,b*f-c*e)),pos.Dot (Vector (f*g-d*i,a*i-c*g, c*d-a*f)),pos.Dot (Vector (d*h-e*g, b*g-a*h, a*e-b*d)))*factor);
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i
}

float QuadTree::GetHeight (Vector Location, Vector & normal) {
  Location = nonlinear_transform->InvTransform (InvScaleTransform (transformation,Location));
  float tmp =  Location.j-root->GetHeight (RootCornerData,Location.i,Location.k,  normal);
  normal = Transform (transformation,nonlinear_transform->TransformNormal (normal));
  normal.Normalize();
  return tmp;
}


void QuadTree::Update (unsigned short numstages, unsigned short whichstage) {
  //GetViewerPosition
  root->Update (RootCornerData,nonlinear_transform->InvTransform (InvScaleTransform (transformation,_Universe->AccessCamera()->GetPosition())),detail,numstages,whichstage);
}

void QuadTree::SetTransformation(const Matrix mat) {
  memcpy (transformation,mat,sizeof(float)*16);
}

static Vector calculatenormscale (const Matrix trans) {
  return Vector (1.0F/(Vector (trans[0],trans[1],trans[2]).Magnitude()),1.0F/(Vector (trans[4],trans[5],trans[6]).Magnitude()),1.0F/(Vector (trans[8],trans[9],trans[10]).Magnitude()));
}

void QuadTree::Render () {
  GFXLoadMatrix (MODEL,transformation);
  GFXBoxInFrustumModel (transformation);
  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXEnable (LIGHTING);
  GFXBlendMode (ONE,ZERO);
  quadsquare::SetCurrentTerrain (&VertexAllocated, &VertexCount, &vertices, &unusedvertices, nonlinear_transform,&textures, calculatenormscale(transformation));
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


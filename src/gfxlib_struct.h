/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _GFXLIB_STRUCT
#define _GFXLIB_STRUCT
#include "gfx/vec.h"

struct GFXVertex // Vertex, Normal, Texture, and Environment
{
  float x;
  float y;
  float z;
  float i;
  float j;
  float k;
  float s,t;
  
  GFXVertex(){}
  GFXVertex(const Vector &vert, const Vector &norm, float s, float t){
    SetVertex(vert);
    SetNormal(norm);
    SetTexCoord(s, t);
  }
  GFXVertex (float x, float y, float z, float i, float j, float k, float s, float t) {this->x=x;this->y=y;this->z=z;this->i=i;this->j=j;this->k=k;this->s=s;this->t=t;}
  GFXVertex &SetTexCoord(float s, float t) {this->s = s; this->t = t; return *this;}
  GFXVertex &SetNormal(const Vector &norm) {i = norm.i; j = norm.j; k = norm.k; return *this;}
  GFXVertex &SetVertex(const Vector &vert) {x = vert.i; y = vert.j; z = vert.k; return *this;}
};
struct GFXColor
{
	float r;
	float g;
	float b;
	float a;
 GFXColor (const Vector &v, float a=1.0) {
    this->r = v.i;
    this->g = v.j;
    this->b = v.k;
    this->a = a;
  }
  GFXColor(float r, float g, float b) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = 1.0;
  }

  GFXColor(float r, float g, float b, float a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }
};

enum LIGHT_TARGET {
  ATTENUATE=1,//important ATTENUATE STAYS AT ONE...for w compat.
  DIFFUSE=2,
  SPECULAR=4,
  AMBIENT=8,
  POSITION=16,
};

class GFXLight {
 protected:
  int target;
  float vect[3];//last is w for positional, otherwise 3 for spec
  union {
    float w;//also may be referred to as vect[3];
    int options;
  };
  float diffuse [4];
  float specular[4];
  float ambient[4];
  float attenuate[3];
 public:
  GFXLight() {
   vect[0]=vect[1]=vect[2]=vect[3]=0;
    attenuate[0]=attenuate[1]=attenuate[2]=0;
    diffuse[0]=diffuse[1]=diffuse[2]=0;//openGL defaults
    specular[0]=specular[1]=specular[2]=0;//openGL defaults
    ambient[0]=ambient[1]=ambient[2]=options=0;
    diffuse[3]=specular[3]=ambient[3]=1;
    target =-1;//physical GL light its saved in
  }
  GFXLight::GFXLight (const bool enabled, const GFXColor &vect, const GFXColor &diffuse= GFXColor (0,0,0,1), const GFXColor &specular=GFXColor (0,0,0,1), const GFXColor &ambient=GFXColor(0,0,0,1), const GFXColor&attenuate=GFXColor(1,0,0));
  void SetProperties (enum LIGHT_TARGET, const GFXColor & color);
  void disable ();
  void enable ();
  bool attenuated ();
  void apply_attenuate (bool attenuated);
};
struct GFXTVertex // transformed vertex
{
	float x;
	float y;
	float z;
	float rhw; // reciprocal homogenous w
	int diffuse;
	int specular;
	float s,t;
	float u,v;
};


/////typedef void *LPDIRECT3DVERTEXBUFFER ;

//Vertexlist object (will be turned into a function... or not)
//Vertexlist object (will be turned into a function... or not)

enum POLYTYPE {
  GFXTRI,
  GFXQUAD,
  GFXLINE,
  GFXTRISTRIP,
  GFXQUADSTRIP,
  GFXTRIFAN,//unsupported in mesh...
  GFXLINESTRIP,//unsupported in mesh...
  GFXPOLY//unsupported in mesh...
};
/**
 *   Meant to be a huge list of individual quads (like light maps) 
 *   that need to be resizable, etc all to be drawn at once.... nice on GL :-) 
 */
class /*GFXDRVAPI*/ GFXQuadList {
  int numVertices;
  int numQuads;
  int * quadassignments;
  GFXVertex * myVertices;
  GFXColor *myColors;
  GFXBOOL isColor;
  int Dirty;
 public:
  GFXQuadList(GFXBOOL color=GFXFALSE);
  ~GFXQuadList();
  void Draw();
  int AddQuad (const GFXVertex *vertices, const GFXColor * colors=NULL);
  void DelQuad (int which);
  void ModQuad (int which, const GFXVertex *vertices, const GFXColor * colors=NULL);
};
class /*GFXDRVAPI*/ GFXVertexList {
  int numVertices;
  GFXVertex *myVertices;
  GFXColor *myColors;
  GFXVertexList * tesslist;
  GLenum *mode;
  int display_list;
  int numlists;
  int *offsets;
  int tessellation;
  int changed;
  int numTris ();
  int numQuads();
  void Init (enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColor *colors, int numlists, int *offsets, bool Mutable,int tess);
  void RefreshDisplayList();
public:
  GFXVertexList();
  void Tess (int );
  inline GFXVertexList(enum POLYTYPE poly, int numVertices, const GFXVertex *vertices,bool Mutable=false, int tess =0){Init (&poly, numVertices, vertices, NULL, 1, &numVertices,Mutable,tess);}
  inline GFXVertexList(enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, int numlists, int *offsets, bool Mutable=false, int tess =0) {
    Init(poly,numVertices,vertices,NULL,numlists,offsets,Mutable, tess);
  }
  inline GFXVertexList(enum POLYTYPE poly, int numVertices, const GFXVertex *vertices,const GFXColor *colors, bool Mutable=false, int tess =0){Init (&poly, numVertices, vertices, colors, 1, &numVertices,Mutable,tess);}
  inline GFXVertexList(enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColor *colors, int numlists, int *offsets, bool Mutable=false, int tess =0) {
    Init(poly,numVertices,vertices,colors, numlists,offsets,Mutable, tess);
  }

  ~GFXVertexList();
  
  GFXTVertex *LockTransformed(); // Stuff to support environment mapping
  void UnlockTransformed();
  GFXVertex *LockUntransformed(); // Stuff to support environment mapping
  void UnlockUntransformed();
  
  GFXBOOL SetNext(GFXVertexList *vlist);
  GFXBOOL Mutate (int offset,  const GFXVertex *vlist,int number, const GFXColor *color=NULL);
  void LoadDrawState();
  void Draw();
  void DrawOnce (){LoadDrawState();Draw();}
  void GetPolys (GFXVertex **vert, int *numPolys, int * numTris);
  GFXBOOL SwapUntransformed();
  GFXBOOL SwapTransformed();
};

struct DrawContext {
  float m[16];
  GFXVertexList *vlist;
  DrawContext() { }
  DrawContext(const float  a[16], GFXVertexList *vl) { memcpy(m, a, sizeof(float[16])); vlist = vl;}
};

struct GFXMaterial
{
	float ar; // ambient rgba, if you don't like these things, ask me to rename them
	float ag;
	float ab;
	float aa;

	float dr; // diffuse rgba
	float dg;
	float db;
	float da;

	float sr; // specular rgba
	float sg;
	float sb;
	float sa;

	float er; // emissive rgba
	float eg;
	float eb;
	float ea;

	float power; // specular power
};


typedef float Matrix[16];

enum TEXTURE_TARGET {
  TEXTURE2D,
  CUBEMAP
};

enum TEXTURE_IMAGE_TARGET {
  TEXTURE_2D,
  CUBEMAP_POSITIVE_X,
  CUBEMAP_NEGATIVE_X,
  CUBEMAP_POSITIVE_Y,
  CUBEMAP_NEGATIVE_Y,
  CUBEMAP_POSITIVE_Z,
  CUBEMAP_NEGATIVE_Z,
};

enum MATRIXMODE{
	MODEL,
	VIEW,
	PROJECTION
};

enum TEXTUREFORMAT {
	DUMMY = 0,
	PALETTE8 = 1,
	RGB16 = 2,
	RGBA16 = 3,
	RGB24 = 4,
	RGBA32 = 5,
	RGB32 = 6
};

enum STATE {
	LIGHTING,
	DEPTHTEST,
	DEPTHWRITE,
	TEXTURE0,
	TEXTURE1,
	CULLFACE
};

enum PRIMITIVE {
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	QUADS,
	POLYGON
};

enum ADDRESSMODE{
	WRAP,
	MIRROR,
	CLAMP,
	BORDER
};

enum BLENDFUNC{
    ZERO            = 1,
	ONE             = 2, 
    SRCCOLOR        = 3,
	INVSRCCOLOR     = 4, 
    SRCALPHA        = 5,
	INVSRCALPHA     = 6, 
    DESTALPHA       = 7,
	INVDESTALPHA    = 8, 
    DESTCOLOR       = 9,
	INVDESTCOLOR    = 10, 
    SRCALPHASAT     = 11,
    CONSTALPHA    = 12, 
    INVCONSTALPHA = 13,
    CONSTCOLOR = 14,
    INVCONSTCOLOR = 15
};
enum FILTER {
  NEAREST=0x0,
  BILINEAR=0x1,
  MIPMAP=0x2,
  TRILINEAR=0x4
};
enum DEPTHFUNC{
	NEVER,LESS,EQUAL, LEQUAL, GREATER, NEQUAL, GEQUAL, ALWAYS
};


//Pick data structures
struct PickData {
  int name;
  int zmin;
  int zmax;

  PickData(int name, int zmin, int zmax) : name(name), zmin(zmin), zmax(zmax) { }
};

#endif

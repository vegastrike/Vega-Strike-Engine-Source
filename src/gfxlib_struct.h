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
/// Vertex, Normal, Texture, and (deprecated) Environment Mapping
struct GFXVertex 
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
//Stores a color (or any 4 valued vector)
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

///important ATTENUATE const STAYS AT ONE...for w compat.
enum LIGHT_TARGET {
  ATTENUATE=1,
  DIFFUSE=2,
  SPECULAR=4,
  AMBIENT=8,
  POSITION=16,
};
///Holds all information for a single light object
class GFXLight {
 protected:
  ///physical GL light its saved in
  int target;
  ///last is w for positional, otherwise 3 for spec
  float vect[3];
  union {
    ///also may be referred to as vect[3];
    float w;
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
///Contains 4 texture coordinates (deprecated)
struct GFXTVertex // transformed vertex
{
	float x;
	float y;
	float z;
  /// reciprocal homogenous w
	float rhw; 
	int diffuse;
	int specular;
	float s,t;
	float u,v;
};



enum POLYTYPE {
  GFXTRI,
  GFXQUAD,
  GFXLINE,
  GFXTRISTRIP,
  GFXQUADSTRIP,
  GFXTRIFAN,
  GFXLINESTRIP,
  GFXPOLY,
  GFXPOINT
};

/**
 *   Meant to be a huge list of individual quads (like light maps) 
 *   that need to be resizable, etc all to be drawn at once.... nice on GL :-) 
 */
class /*GFXDRVAPI*/ GFXQuadList {
  ///Num vertices currently _allocated_ on quad list 
  int numVertices;
  ///Number of quads to be drawn packed first numQuads*4 vertices
  int numQuads;
  ///Assignments to packed data for quad modification
  int * quadassignments;
  ///all numVertices allocated vertices and color
  GFXVertex * myVertices;   GFXColor *myColors;
  ///Is color in this quad list
  GFXBOOL isColor;
  ///number of "dirty" quads, hence gaps in quadassignments that must be assigned before more are allocated
  int Dirty;
 public:
  ///Creates an initial Quad List
  GFXQuadList(GFXBOOL color=GFXFALSE);
  ///Trashes given quad list
  ~GFXQuadList();
  ///Draws all quads contained in quad list
  void Draw();
  ///Adds quad to quad list, an integer indicating number that should be used to henceforth Mod it or delete it
  int AddQuad (const GFXVertex *vertices, const GFXColor * colors=NULL);
  ///Removes quad from Quad list
  void DelQuad (int which);
  ///modifies quad in quad list to contain new vertices and color information
  void ModQuad (int which, const GFXVertex *vertices, const GFXColor * colors=NULL);
};
/**
 * A vertex list is a list of any conglomeration of POLY TYPES.
 * It is held for storage in an array of GFXVertex but attempts
 * to use a display list to hold information if possible 
 */
class /*GFXDRVAPI*/ GFXVertexList {
  ///Num vertices allocated
  int numVertices;
  ///Vertices and colors stored 
  GFXVertex *myVertices;  GFXColor *myColors;
  ///Tesselations of current vertex list (deprecated)
  GFXVertexList * tesslist;
  ///Array of modes that vertices will be drawn with
  GLenum *mode;
  ///Display list number if list is indeed active. 0 otherwise
  int display_list;
  ///number of different mode, drawn lists
  int numlists;
  /**
   * number of vertices in each individual mode.
   * 2 triangles 3 quads and 2 lines would be {6,12,4} as the offsets
   */
  int *offsets;
  ///Tesselation level (FIXME, Deprecated);
  int tessellation;
  ///If vertex list has been mutated since last draw
  int changed;
  ///Returns number of Triangles in vertex list (counts tri strips)
  int numTris ();
  ///Returns number of Quads in vertex list (counts quad strips)
  int numQuads();
  ///Init function (call from construtor)
  void Init (enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColor *colors, int numlists, int *offsets, bool Mutable,int tess);
  ///Propagates modifications to the display list
  void RefreshDisplayList();
public:
  GFXVertexList();
  ///Tesselates to a certain extent.
  void Tess (int );
  ///creates a vertex list with 1 polytype and a given number of vertices
  inline GFXVertexList(enum POLYTYPE poly, int numVertices, const GFXVertex *vertices,bool Mutable=false, int tess =0){Init (&poly, numVertices, vertices, NULL, 1, &numVertices,Mutable,tess);}
  ///Creates a vertex list with an arbitrary number of poly types and given vertices, num list and offsets (see above)
  inline GFXVertexList(enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, int numlists, int *offsets, bool Mutable=false, int tess =0) {
    Init(poly,numVertices,vertices,NULL,numlists,offsets,Mutable, tess);
  }
  ///Creates a vertex list with 1 poly type and color information to boot
  inline GFXVertexList(enum POLYTYPE poly, int numVertices, const GFXVertex *vertices,const GFXColor *colors, bool Mutable=false, int tess =0){Init (&poly, numVertices, vertices, colors, 1, &numVertices,Mutable,tess);}
  ///Creates a vertex list with an arbitrary number of poly types and color
  inline GFXVertexList(enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColor *colors, int numlists, int *offsets, bool Mutable=false, int tess =0) {
    Init(poly,numVertices,vertices,colors, numlists,offsets,Mutable, tess);
  }
  ~GFXVertexList();

  ///Deprecated. Should use with glLockArrays
  GFXVertex *Lock();   void Unlock();
  ///Returns the array of vertices to be mutated
  GFXVertex * BeginMutate (int offset) {return &myVertices[offset];}
  ///Ends mutation and refreshes display list
  void EndMutate () {RefreshDisplayList();}
  ///Loads the draw state of a given vlist for mass drawing
  void LoadDrawState();
  ///Draws a single copy of the mass-loaded vlist
  void Draw();
  ///Loads draw state and prepares to draw only once
  void DrawOnce (){LoadDrawState();Draw();}
  ///returns a packed vertex list with number of polys and number of tries to passed in arguments. Useful for getting vertex info from a mesh
  void GetPolys (GFXVertex **vert, int *numPolys, int * numTris);
};

/**
 * Stores the Draw Context that a vertex list might be drawn with.
 * Especially useful for mass queued drawing (load matrix, draw... )
 */
struct DrawContext {
  float m[16];
  GFXVertexList *vlist;
  DrawContext() { }
  DrawContext(const float  a[16], GFXVertexList *vl) { memcpy(m, a, sizeof(float[16])); vlist = vl;}
};

/**
 * holds all parameters for materials
 */
struct GFXMaterial
{
  /// ambient rgba, if you don't like these things, ask me to rename them
	float ar;float ag;float ab;float aa;
  /// diffuse rgba
	float dr;float dg;float db;float da;
  /// specular rgba
	float sr;float sg;float sb;float sa;
  /// emissive rgba
	float er;float eg;float eb;float ea;
  /// specular power
	float power; 
};

///Column major order Matrix
typedef float Matrix[16];

//Textures may only be cube maps or Texture2d
enum TEXTURE_TARGET {
  TEXTURE2D,
  CUBEMAP
};
///Textures may only be cube maps or Texture2d
enum TEXTURE_IMAGE_TARGET {
  TEXTURE_2D,
  CUBEMAP_POSITIVE_X,
  CUBEMAP_NEGATIVE_X,
  CUBEMAP_POSITIVE_Y,
  CUBEMAP_NEGATIVE_Y,
  CUBEMAP_POSITIVE_Z,
  CUBEMAP_NEGATIVE_Z,
};

/**
 * Unlike OpenGL, 3 matrices are saved, the Model, the View and the Projection
 * Consistency is maintained through pushing the rotation part of view 
 * onto projection matrix
 */
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
/**
 * The following state may be activated/deactivated
 * LIGHTING, DEPTHTEST,DEPTHWRITE, TEXTURE0, and TEXTURE1.  Future support
 * For arbitrary number of texture units should be added FIXME
 */
enum STATE {
	LIGHTING,
	DEPTHTEST,
	DEPTHWRITE,
	TEXTURE0,
	TEXTURE1,
	CULLFACE
};

/**
 * Address modes various textures may have
 */
enum ADDRESSMODE{
	WRAP,
	MIRROR,
	CLAMP,
	BORDER
};
/**
 * Blend functions that a blendmode may have Not all work for all systems
 */
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

/** 
 * Different types of filtering. 
 * Bilinear and mipmap are independent params (hence powers of two)
 */
enum FILTER {
  NEAREST=0x0,
  BILINEAR=0x1,
  MIPMAP=0x2,
  TRILINEAR=0x4
};

/**
 * Used for depth and alpha comparisons
 */
enum DEPTHFUNC{
	NEVER,LESS,EQUAL, LEQUAL, GREATER, NEQUAL, GEQUAL, ALWAYS
};


///Pick data structures
struct PickData {
  int name;
  int zmin;
  int zmax;

  PickData(int name, int zmin, int zmax) : name(name), zmin(zmin), zmax(zmax) { }
};

#endif

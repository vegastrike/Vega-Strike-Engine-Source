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
#ifndef D3D_STRUCT
#define D3D_STRUCT

#include "gfx_transform_vector.h"

struct GFXVertex // Vertex, Normal, Texture, and Environment
{
	float x;
	float y;
	float z;
	float i;
	float j;
	float k;
	//DWORD diffuse;
	//DWORD specular;
	float s,t;
	float u,v;

	GFXVertex()
	{
	};
	GFXVertex(const Vector &vert, const Vector &norm, float s, float t)
	{
		SetVertex(vert);
		SetNormal(norm);
		SetTexCoord(s, t);
	}
	GFXVertex &SetTexCoord(float s, float t) {this->s = s; this->t = t; return *this;}
	GFXVertex &SetNormal(const Vector &norm) {i = norm.i; j = norm.j; k = norm.k; return *this;}
	GFXVertex &SetVertex(const Vector &vert) {x = vert.i; y = vert.j; z = vert.k; return *this;}
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


class /*GFXDRVAPI*/ GFXVertexList {


	// Untransformed and transformed data 
  /////FIXME VEGASTRIKEvoid* lpd3dvbUntransformed;
  /////FIXME VEGASTRIKEvoid* lpd3dvbTransformed;
	int numVertices;
	int numTriangles;
	int numQuads;
	GFXVertex *myVertices;
	GFXVertexList *next;
	//VLISTTYPE ptype;

	int display_list;

public:
	GFXVertexList();
	GFXVertexList(int numVertices,int numTriangle, int numQuad, GFXVertex *vertices);
	~GFXVertexList();

	GFXTVertex *LockTransformed(); // Stuff to support environment mapping
	void UnlockTransformed();

	GFXVertex *LockUntransformed(); // Stuff to support environment mapping
	void UnlockUntransformed();

	BOOL SetNext(GFXVertexList *vlist);

	BOOL Draw();
	BOOL SwapUntransformed();
	BOOL SwapTransformed();
};

class GFXQuadstrip {


	GFXVertex *myVertices;
	int numVertices;

	int display_list;

public:
	GFXQuadstrip() : myVertices(0), numVertices(0), display_list(0) { }
	GFXQuadstrip(int numVertices, GFXVertex *vertices);
	~GFXQuadstrip();

	GFXTVertex *LockTransformed(); // Stuff to support environment mapping
	void UnlockTransformed();

	GFXVertex *LockUntransformed(); // Stuff to support environment mapping
	void UnlockUntransformed();

	BOOL Draw();
	BOOL SwapUntransformed();
	BOOL SwapTransformed();
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

struct GFXColor
{
	float r;
	float g;
	float b;
	float a;

	GFXColor(float r, float g, float b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 1.0;
	}

	GFXColor(float r, float g, float b, float a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
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
	BOTHSRCALPHA    = 12, 
    BOTHINVSRCALPHA = 13,
	FORCE_DWORD   = 0x7fffffff, 
};

enum DEPTHFUNC{
	NEVER,LESS,EQUAL, LEQUAL, GREATER, NEQUAL, GEQUAL, ALWAYS
};



#endif

/**
 *
 * Data structures for quadtree terrain storage.
 * This code may be freely modified and redistributed.  I make no
 * warrantees about it; use at your own risk.  If you do incorporate
 * this code into a project, I'd appreciate a mention in the credits.
 * Thatcher Ulrich <tu@tulrich.com>
 *
 * Rewritten and adapted to Vegastrike by Daniel Horn
 */


#ifndef QUADTREE_H
#define QUADTREE_H
#include "vec.h"
#include "gfxlib.h"
#include <vector>

struct Texture;
struct TerrainTexture {
  BLENDFUNC blendSrc;
  BLENDFUNC blendDst;
  int color;
  GFXMaterial material;
  bool reflect;
  union {
    char * filename;
    Texture * t;
  }tex;
  TerrainTexture () {
    tex.filename = NULL;
    GFXGetMaterial (0, material);//by default it's the default material;
  }
};

struct HeightMapInfo {
	short*	Data;
	int	XOrigin, ZOrigin;
	int	XSize, ZSize;
	int	RowWidth;
	int	Scale;

	float	Sample(int x, int z) const;
};
/**
 * This has 4 lists of indices, one for "numbers of corners" a triangle may be filled 
 * If the trianlge has 0 vertices filled, it is a nonblended one with all 4 filled
 */
struct TextureIndex {
  std::vector <unsigned int> q;
  std::vector <GFXColorVertex> c;
  void Clear();
};

/**
 * We could make it virtual and ahve a sphere-map or cube-map version of this
 *
 */
class IdentityTransform {
 public:
  ///Transforms in a possibly nonlinear way the vector to some new space
  virtual Vector Transform (const Vector &);
  ///Transforms in reverse the vector into quadsquare space
  virtual Vector InvTransform (const Vector &);
  ///Transforms a min and a max vector and figures out what is bigger
  virtual void TransformBox (Vector &min, Vector &max);
  float TransformS (float x);
  float TransformT (float y);
};

struct	VertInfo {
  unsigned short Y;
  unsigned char Tex;
  unsigned char Rem;
  
  unsigned int vertindex;
  void SetTex(float);
  unsigned short GetTex() const;
};


class quadsquare;

/**
 * A structure used during recursive traversal of the tree to hold
 * relevant but transitory data.
 */
struct quadcornerdata {
	const quadcornerdata*	Parent;
	quadsquare*	Square;
	int	ChildIndex;
	int	Level;
	int	xorg, zorg;
	VertInfo	Verts[4];	// ne, nw, sw, se
};

/**
 * A node in the quad tree
 * holds its own relevant vertex data (middle and either even or odd 4 sets of data (corners or diagonals
 * Keeps track of its errors and children as well
 */
class quadsquare {
 public:
	quadsquare*	Child[4];

	VertInfo	Vertex[5];	// center, e, n, w, s
	unsigned short	Error[6];	// e, s, children: ne, nw, sw, se
	unsigned short	MinY, MaxY;	// Bounds for frustum culling and error testing.
	unsigned char	EnabledFlags;	// bits 0-7: e, n, w, s, ne, nw, sw, se
	unsigned char	SubEnabledCount[2];	// e, s enabled reference counts.
	bool	Static;
	bool	Dirty;	// Set when vertex data has changed, but error/enabled data has not been recalculated.

// public:
	quadsquare(quadcornerdata* pcd);
	~quadsquare();

	void	AddHeightMap(const quadcornerdata& cd, const HeightMapInfo& hm);
	void	StaticCullData(const quadcornerdata& cd, float ThresholdDetail);	
	float	RecomputeErrorAndLighting(const quadcornerdata& cd);
	int	CountNodes();
  ///Make sure to translate into Quadtree Space
	void	Update(const quadcornerdata& cd, const Vector &ViewerLocation, float Detail);
	int	Render(const quadcornerdata& cd);

	float	GetHeight(const quadcornerdata& cd, float x, float z, Vector & normal);
	static Vector MakeLightness (float xslope, float zslope);
  static void SetCurrentTerrain (unsigned int * VertexAllocated, unsigned int * VertexCount, GFXVertexList *vertices, std::vector <unsigned int> *unusedvertices, IdentityTransform * transform, std::vector <TerrainTexture> *texturelist );
	
private:
  static void tri (unsigned int Aind, unsigned short Atex, unsigned int Bind, unsigned short Btex, unsigned int Cind, unsigned short Ctex);
  ///Sets the 5 vertices in vertexs array in 3space from a quadcornerdata and return half of the size
  unsigned int SetVertices (GFXVertex * vertexs, const quadcornerdata &pcd);
	void	EnableEdgeVertex(int index, bool IncrementCount, const quadcornerdata& cd);
	quadsquare*	EnableDescendant(int count, int stack[], const quadcornerdata& cd);
	void	EnableChild(int index, const quadcornerdata& cd);
	void	NotifyChildDisable(const quadcornerdata& cd, int index);

	void	ResetTree();
	void	StaticCullAux(const quadcornerdata& cd, float ThresholdDetail, int TargetLevel);

	quadsquare*	GetNeighbor(int dir, const quadcornerdata& cd);
	void	CreateChild(int index, const quadcornerdata& cd);
	void	SetupCornerData(quadcornerdata* q, const quadcornerdata& pd, int ChildIndex);

	void	UpdateAux(const quadcornerdata& cd, const Vector &ViewerLocation, float CenterError);
	void	RenderAux(const quadcornerdata& cd, CLIPSTATE vis);
	void	SetStatic(const quadcornerdata& cd);
	static IdentityTransform * nonlinear_trans;
	static unsigned int * VertexAllocated;
	static unsigned int * VertexCount;
	static GFXVertexList *vertices;
	static GFXVertexList *blendVertices;
	static std::vector <unsigned int> *unusedvertices;
	static std::vector <TerrainTexture> *textures;
	static std::vector <TextureIndex> indices;
};


#endif // QUADTREE_HPP

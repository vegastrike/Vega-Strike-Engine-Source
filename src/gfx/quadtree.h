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
struct HeightMapInfo {
	short*	Data;
	int	XOrigin, ZOrigin;
	int	XSize, ZSize;
	int	RowWidth;
	int	Scale;

	float	Sample(int x, int z) const;
};


struct	VertInfo {
  float	Y;
  unsigned int vertindex;
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


struct quadsquare {
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

	float	GetHeight(const quadcornerdata& cd, float x, float z);
  static void SetCurrentTerrain (unsigned int * VertexAllocated, unsigned int * VertexCount, GFXVertexList *vertices, std::vector <unsigned int> *unusedvertices);
	
private:
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
  static unsigned int * VertexAllocated;
  static unsigned int * VertexCount;
  static GFXVertexList *vertices;
  static std::vector <unsigned int> *unusedvertices;
};


#endif // QUADTREE_HPP

/*
    Copyright (C) 2000 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
-------------------------------------------------------------------------
*
*           OPCODE collision detection plugin for CrystalSpace
*
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*
-------------------------------------------------------------------------
*/

#include "Stdafx.h"
#include "qsqrt.h"
#include "qint.h"
#include "garray.h"
//#include "csutil/dirtyaccessarray.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "CSopcodecollider.h"
//#include "igeom/polymesh.h"
//#include "igeom/trimesh.h"
#include "collider.h"
//#include "gfx/mesh.h"
//#include "OPC_TreeBuilders.h"


using namespace Opcode;


static CS_DECLARE_GROWING_ARRAY_REF (pairs,csCollisionPair);

//int csOPCODECollider::numHits = 0;


/*csOPCODECollider::csOPCODECollider (iTriangleMesh* mesh) 
{
  m_pCollisionModel = 0;
  indexholder = 0;
  vertholder = 0;
  //transform.m[0][3] = 0;
  //transform.m[1][3] = 0;
  //transform.m[2][3] = 0;
  //transform.m[3][3] = 1;

  opcMeshInt.SetCallback (&MeshCallback, this);

  GeometryInitialize (mesh);
}
*/
/*csOPCODECollider::csOPCODECollider (iPolygonMesh* mesh) 
{
  m_pCollisionModel = 0;
  indexholder = 0;
  vertholder = 0;
  //transform.m[0][3] = 0;
  //transform.m[1][3] = 0;
  //transform.m[2][3] = 0;
  //transform.m[3][3] = 1;

  opcMeshInt.SetCallback (&MeshCallback, this);

  GeometryInitialize (mesh);
}
*/
csOPCODECollider::csOPCODECollider (const std::vector <bsp_polygon> &polygons) 
{
  m_pCollisionModel = 0;
  indexholder = 0;
  vertholder = 0;
  TreeCollider.SetFirstContact(false);
  TreeCollider.SetFullBoxBoxTest(false);
  TreeCollider.SetTemporalCoherence(true);
  opcMeshInt.SetCallback (&MeshCallback, this);

  GeometryInitialize (polygons);
}

inline float min3 (float a, float b, float c)
{ return (a < b ? (a < c ? a : (c < b ? c : b)) : (b < c ? b : c)); }
inline float max3(float a, float b, float c)
{ return (a > b ? (a > c ? a : (c > b ? c : b)) : (b > c ? b : c)); }

/*void csOPCODECollider::GeometryInitialize (csVector3* vertices,
    size_t vertcount, csTriangle* triangles, size_t tri_count)
{
  OPCODECREATE OPCC;
  size_t i;

  if (tri_count>=1)
  {
    m_pCollisionModel = new Opcode::Model;
    if (!m_pCollisionModel)
      return;

    vertholder = new Point [vertcount];
    indexholder = new unsigned int[3*tri_count];

    csBox3 tmp;
    tmp.StartBoundingBox ();
    for (i = 0; i < vertcount; i++)
    {
      tmp.AddBoundingVertex (vertices[i]);
      vertholder[i].Set (vertices[i].x , vertices[i].y , vertices[i].z);
    }

    radius = max3 (tmp.MaxX ()- tmp.MinX (), tmp.MaxY ()- tmp.MinY (),
	tmp.MaxZ ()- tmp.MinZ ());

    int index = 0;
    for (i = 0 ; i < tri_count ; i++)
    {
      indexholder[index++] = triangles[i].a;
      indexholder[index++] = triangles[i].b;
      indexholder[index++] = triangles[i].c;
    }

    opcMeshInt.SetNbTriangles (tri_count);
    opcMeshInt.SetNbVertices (vertcount);

    // Mesh data
    OPCC.mIMesh = &opcMeshInt;
    OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
    OPCC.mNoLeaf = true;
    OPCC.mQuantized = true;
    OPCC.mKeepOriginal = false;
    OPCC.mCanRemap = false;
  }
  else
    return;

  // this should create the OPCODE model
  bool status = m_pCollisionModel->Build (OPCC);
  if (!status) { return; };
}
*/
void csOPCODECollider::GeometryInitialize (const std::vector <bsp_polygon> &polygons )
{
  OPCODECREATE OPCC;
  size_t i;
  int  tri_count = (int)polygons.size();

  if (tri_count>=1)
  {
    m_pCollisionModel = new Opcode::Model;
    if (!m_pCollisionModel)
      return;

    vertholder = new Point [3*tri_count];
    indexholder = new unsigned int[3*tri_count];

    csBox3 tmp;
    tmp.StartBoundingBox ();
    for (i = 0; i < tri_count; ++i)
    {
	  const bsp_polygon *p = (&polygons[i]);
      tmp.AddBoundingVertex (p->v[0]);
	  tmp.AddBoundingVertex (p->v[1]);
	  tmp.AddBoundingVertex (p->v[2]);
	  vertholder[0].Set (p->v[0].i , p->v[0].j , p->v[0].k);
	  vertholder[1].Set (p->v[1].i , p->v[1].j , p->v[1].k);
	  vertholder[2].Set (p->v[2].i , p->v[2].j , p->v[2].k);
    }

    radius = max3 (tmp.MaxX ()- tmp.MinX (), tmp.MaxY ()- tmp.MinY (),
	tmp.MaxZ ()- tmp.MinZ ());

    int index = 0;
    for (i = 0 ; i < tri_count ; i++)
    {
	  
      indexholder[index++] = 0 + i*3;
      indexholder[index++] = 1 + i*3;
      indexholder[index++] = 2 + i*3;
    }

    opcMeshInt.SetNbTriangles (tri_count);
    opcMeshInt.SetNbVertices (tri_count * 3);

    // Mesh data
    OPCC.mIMesh = &opcMeshInt;
    OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
    OPCC.mNoLeaf = true;
    OPCC.mQuantized = true;
    OPCC.mKeepOriginal = false;
    OPCC.mCanRemap = false;
  }
  else
    return;

  // this should create the OPCODE model
  bool status = m_pCollisionModel->Build (OPCC);
  if (!status) { return; };
}

/*void csOPCODECollider::GeometryInitialize (iTriangleMesh* mesh)
{
  // first, count the number of triangles polyset contains
  csVector3* vertices = mesh->GetVertices ();
  size_t vertcount = mesh->GetVertexCount ();
  csTriangle* triangles = mesh->GetTriangles ();
  size_t tri_count = mesh->GetTriangleCount ();
  GeometryInitialize (vertices, vertcount, triangles, tri_count);
}
*/

/*void csOPCODECollider::GeometryInitialize (iPolygonMesh* mesh)
{
  // first, count the number of triangles polyset contains
  csVector3* vertices = mesh->GetVertices ();
  size_t vertcount = mesh->GetVertexCount ();
  csTriangle* triangles = mesh->GetTriangles ();
  size_t tri_count = mesh->GetTriangleCount ();
  GeometryInitialize (vertices, vertcount, triangles, tri_count);
}
*/
csOPCODECollider::~csOPCODECollider ()
{
  if (m_pCollisionModel)
  {
    delete m_pCollisionModel;
    m_pCollisionModel = 0;
  }

  delete[] indexholder;
  delete[] vertholder;
}

void csOPCODECollider::MeshCallback (udword triangle_index,
				     VertexPointers& triangle,
				     void* user_data)
{
  csOPCODECollider* collider = (csOPCODECollider*)user_data;

  udword *tri_array = collider->indexholder;
  Point *vertholder = collider->vertholder;
  int index = 3 * triangle_index;
  triangle.Vertex[0] = &vertholder [tri_array[index]] ;
  triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
  triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
}

bool csOPCODECollider::Collide( csOPCODECollider &otherCollider, 
								const csReversibleTransform *trans1,
								const csReversibleTransform *trans2)
{
	csOPCODECollider* col2 = (csOPCODECollider*) &otherCollider;
	
	ColCache.Model0 = this->m_pCollisionModel;
	ColCache.Model1 = col2->m_pCollisionModel;
	csMatrix3 m1;
	if (trans1) m1 = trans1->GetT2O ();
	csMatrix3 m2;
	if (trans2) m2 = trans2->GetT2O ();
	csVector3 u;
	Matrix4x4 transform1;
  	transform1.m[0][3] = 0;
    transform1.m[1][3] = 0;
	transform1.m[2][3] = 0;
	transform1.m[3][3] = 1;
	Matrix4x4 transform2;
	transform2.m[0][3] = 0;
	transform2.m[1][3] = 0;
	transform2.m[2][3] = 0;
	transform2.m[3][3] = 1;
  	u = m1.Row1 ();
    transform1.m[0][0] = u.x;
	transform1.m[1][0] = u.y;
	transform1.m[2][0] = u.z;
	u = m2.Row1 ();
	transform2.m[0][0] = u.x;
	transform2.m[1][0] = u.y;
	transform2.m[2][0] = u.z;
	u = m1.Row2 ();
  	transform1.m[0][1] = u.x; 
    transform1.m[1][1] = u.y; 
	transform1.m[2][1] = u.z; 
	u = m2.Row2 (); 
	transform2.m[0][1] = u.x; 
	transform2.m[1][1] = u.y; 
	transform2.m[2][1] = u.z; 
	u = m1.Row3 (); 
	transform1.m[0][2] = u.x; 
	transform1.m[1][2] = u.y; 
	transform1.m[2][2] = u.z; 
	u = m2.Row3(); 
	transform2.m[0][2] = u.x; 
	transform2.m[1][2] = u.y; 
	transform2.m[2][2] = u.z; 
	if (trans1) u = trans1->GetO2TTranslation (); 
	else u.Set (0, 0, 0); 
	transform1.m[3][0] = u.x; 
	transform1.m[3][1] = u.y; 
	transform1.m[3][2] = u.z; 
			   
	if (trans2) u = trans2->GetO2TTranslation (); 
	else u.Set (0, 0, 0); 
	transform2.m[3][0] = u.x; 
	transform2.m[3][1] = u.y; 
	transform2.m[3][2] = u.z; 
					 
	if (TreeCollider.Collide (ColCache, &transform1, &transform2)) { 
		bool status = (TreeCollider.GetContactStatus () != FALSE); 
		if (status)  { 
			CopyCollisionPairs (this, col2); 
//			++csOPCODECollider::numHits;
		} 
		return(status); 
	} else 
		return(false);
}

csCollisionPair* csOPCODECollider::GetCollisions()
{
	return(pairs.GetArray());
}

size_t csOPCODECollider::GetCollisionPairCount()
{
	return(pairs.Length());
}

void csOPCODECollider::ResetCollisionPairs()
{
	pairs.SetLength(0);
}

void csOPCODECollider::SetOneHitOnly(bool on)
{
	TreeCollider.SetFirstContact(on);
}
bool csOPCODECollider::GetOneHitOnly()
{
	return(TreeCollider.FirstContactEnabled() != FALSE);
}

Vector csOPCODECollider::getVertex(unsigned int which) const 
{
	const MeshInterface *tmp = m_pCollisionModel->GetMeshInterface();
	VertexPointers vertp;
	tmp->GetTriangle(vertp,0);
	const Point *tmpPoint = vertp.Vertex[which];
	const float f[3] = {tmpPoint->x,tmpPoint->y,tmpPoint->z};
	return(Vector(f[0],f[1],f[2]));
}

/*Vector csOPCODECollider::getVertex(unsigned int which) const {
	// Access mesh interface, send which to get correct vertex, 
	unsigned int k = which / 3;
	const MeshInterface *tmp = m_pCollisionModel->GetMeshInterface();
	unsigned int tmp3 = tmp->GetNbTriangles();
	if(!tmp3){
		return(Vector(0,0,0));
	}
	if(k>=tmp3)
		k = tmp3 - 1;
	const IndexedTriangle  *a = tmp->GetTris();
	csVector3 b(a[k].mVRef[0],a[k].mVRef[1],a[k].mVRef[2]);
	return(Vector(b.x,b.y,b.z));
	
	switch (which%3){
		case 0:
			return b.p1;
		case 1:
			return b.p2;
		default:
			return b.p3;
	}
}
*/

void csOPCODECollider::CopyCollisionPairs(csOPCODECollider* col1, 
										  csOPCODECollider* col2)
{
  int size = (int) (udword(TreeCollider.GetNbPairs ()));
  if (size == 0) return;
  int N_pairs = size;
  const Pair* colPairs=TreeCollider.GetPairs ();
  Point* vertholder0 = col1->vertholder;
  if (!vertholder0) return;
  Point* vertholder1 = col2->vertholder;
  if (!vertholder1) return;
  if (!vertholder1) return;
  udword* indexholder0 = col1->indexholder;
  if (!indexholder0) return;
  udword* indexholder1 = col2->indexholder;
  if (!indexholder1) return;
  Point* current;
  int i, j;
  size_t oldlen = pairs.Length ();
  pairs.SetLength (oldlen + N_pairs);


  for (i = 0 ; i < N_pairs ; i++){
    j = 3 * colPairs[i].id0;
	current = &vertholder0[indexholder0[j]];
	pairs[oldlen].a1 = csVector3 (current->x, current->y, current->z);
    current = &vertholder0[indexholder0[j + 1]];
	pairs[oldlen].b1 = csVector3 (current->x, current->y, current->z);
	current = &vertholder0[indexholder0[j + 2]];
	pairs[oldlen].c1 = csVector3 (current->x, current->y, current->z);
	
	j = 3 * colPairs[i].id1;
	current = &vertholder1[indexholder1[j]];
	pairs[oldlen].a2 = csVector3 (current->x, current->y, current->z);
	current = &vertholder1[indexholder1[j + 1 ]];
	pairs[oldlen].b2 = csVector3 (current->x, current->y, current->z);
	current = &vertholder1[indexholder1[j + 2 ]];
	pairs[oldlen].c2 = csVector3 (current->x, current->y, current->z);
	
	++oldlen;			
  }
}

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
#include "opcodeqsqrt.h"
#include "opcodeqint.h"
#include "opcodegarray.h"
#define _X 1000
#include "CSopcodecollider.h"
#undef _X

using namespace Opcode;

static CS_DECLARE_GROWING_ARRAY_REF (pairs,csCollisionPair);

csOPCODECollider::csOPCODECollider (const std::vector <bsp_polygon> &polygons)
{
	m_pCollisionModel = 0;
	indexholder = 0;
	vertholder = 0;
	pairs.IncRef();
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

void csOPCODECollider::GeometryInitialize (const std::vector <bsp_polygon> &polygons )
{
	OPCODECREATE OPCC;
	int  tri_count = 0;
	int  vert_count = 0;
	for(int i = 0; i <(int)polygons.size();++i) {
		vert_count += polygons[i].v.size();
	}
	tri_count = vert_count / 3;
	if (tri_count) {
		m_pCollisionModel = new Opcode::Model;
		if (!m_pCollisionModel)
			return;

		vertholder = new Point [vert_count];
		indexholder = new unsigned int[vert_count];

		csBox3 tmp;
		tmp.StartBoundingBox ();
		int last = 0;
		int index = 0;
		for (int i = 0; i < (int)polygons.size(); ++i) {
			const bsp_polygon *p = (&polygons[i]);
			for(int j = 0; j < (int)p->v.size();++j) {
				vertholder[last++].Set (p->v[j].i , p->v[j].j , p->v[j].k);
				tmp.AddBoundingVertex (p->v[j]);
				indexholder[index++] = last-1;
			}
		}
		radius = max3 (tmp.MaxX ()- tmp.MinX (), tmp.MaxY ()- tmp.MinY (),
			tmp.MaxZ ()- tmp.MinZ ());
		opcMeshInt.SetNbTriangles (tri_count);
		opcMeshInt.SetNbVertices (last);

		// Mesh data
		OPCC.mIMesh = &opcMeshInt;
		OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
		OPCC.mNoLeaf = true;
		OPCC.mQuantized = true;
		OPCC.mKeepOriginal = false;
		OPCC.mCanRemap = false;
	} else
	return;

	// this should create the OPCODE model
	bool status = m_pCollisionModel->Build (OPCC);
}


csOPCODECollider::~csOPCODECollider ()
{
	if (m_pCollisionModel) {
		delete m_pCollisionModel;
		m_pCollisionModel = 0;
	}
	pairs.DecRef();
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
	//	ResetCollisionPairs();
	if (TreeCollider.Collide (ColCache, &transform1, &transform2)) {
		bool status = (TreeCollider.GetContactStatus () != FALSE);
		if (status) {
			CopyCollisionPairs (this, col2);
		}
		return(status);
	} else
	return(false);
}


csCollisionPair *csOPCODECollider::GetCollisions()
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
	// This function is used to position the damage particles
	// Turning this off for now so it doesn't affect perf
	return(Vector(0,0,0));
	/*
		unsigned int k = which / 3;
		unsigned int nt = 0;
		const MeshInterface *tmp = m_pCollisionModel->GetMeshInterface();
		VertexPointers vertp;
		if( (nt = tmp->GetNbTriangles()) == 0)
			return(Vector(0,0,0));
		if(k >= nt ) k = nt-1;
		tmp->GetTriangle(vertp,k);
		const Point *tmpPoint = vertp.Vertex[which%3];
		const float f[3] = {tmpPoint->x,tmpPoint->y,tmpPoint->z};
		return(Vector(f[0],f[1],f[2]));
	*/
}


void csOPCODECollider::CopyCollisionPairs(csOPCODECollider* col1,
csOPCODECollider* col2)
{
	if(!col1 || !col2) return;

	int N_pairs = (int) (udword(TreeCollider.GetNbPairs ()));
	if (N_pairs == 0) return;

	const Pair* colPairs=TreeCollider.GetPairs ();
	Point* vertholder0 = col1->vertholder;
	Point* vertholder1 = col2->vertholder;
	udword* indexholder0 = col1->indexholder;
	udword* indexholder1 = col2->indexholder;
	Point* current;
	int i, j;
	size_t oldlen = pairs.Length ();
	pairs.SetLength (oldlen + N_pairs);

	for (i = 0 ; i < N_pairs ; ++i) {
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

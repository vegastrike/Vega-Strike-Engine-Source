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

#ifndef __CS_OPCODECOL_H__
#define __CS_OPCODECOL_H__

#include "Stdafx.h"
#include "csgeom2/opmatrix3.h"
#include "csgeom2/opvector3.h"
#include "csgeom2/opbox.h"
#include "csgeom2/optri.h"
#include "csgeom2/optransfrm.h"
#include "opcodegarray.h"
#include "basecollider.h"
#include "gfx/mesh.h"

/// Low level collision detection using Opcode library.
class csOPCODECollider :
  public iCollider
{
private:
	void GeometryInitialize (const std::vector <bsp_polygon> &polygons);
	static void MeshCallback (udword triangle_index, 
    Opcode::VertexPointers& triangle, void* user_data);
	float radius;
	Opcode::Model* m_pCollisionModel;
	unsigned int* indexholder;
	Opcode::Point *vertholder;
	Opcode::MeshInterface opcMeshInt;

public:
	csOPCODECollider (const std::vector <bsp_polygon> &polygons);
  	virtual ~csOPCODECollider ();
  	Opcode::AABBTreeCollider TreeCollider;
  	Opcode::BVTCache ColCache;
  
	int GetColliderType () {return CS_MESH_COLLIDER;}

  	bool Collide (csOPCODECollider &pOtherCollider,
    const csReversibleTransform *pThisTransform = 0,
    const csReversibleTransform *pOtherTransform = 0);

  	/// Query the array with collisions (and their count).
  	static csCollisionPair *GetCollisions ();

  	static void ResetCollisionPairs ();
  	static size_t GetCollisionPairCount();
  
  	void SetOneHitOnly (bool fh);
  	bool GetOneHitOnly ();
  	float GetRadius () {return radius;};
  
  	void CopyCollisionPairs (csOPCODECollider* col1, csOPCODECollider* col2);
	  
	Vector getVertex (unsigned int which) const;
 	unsigned int getNumVertex() { return(m_pCollisionModel->GetMeshInterface()->GetNbVertices());}
};

#endif // __CS_OPCODECOL_H__

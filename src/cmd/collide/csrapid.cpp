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

#include "cssysdef.h"
#include "csrapid.h"
#include "rapcol.h"
#include "ivaria/polymesh.h"

CS_IMPLEMENT_PLUGIN

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csRapidCollideSystem)
  SCF_IMPLEMENTS_INTERFACE (iCollideSystem)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRapidCollideSystem::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csRapidCollideSystem)

SCF_EXPORT_CLASS_TABLE (rapid)
  SCF_EXPORT_CLASS (csRapidCollideSystem,
    "crystalspace.collisiondetection.rapid",
    "Crystal Space RAPID CD System")
SCF_EXPORT_CLASS_TABLE_END

csRapidCollideSystem::csRapidCollideSystem (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csRapidCollideSystem::~csRapidCollideSystem ()
{
}

iCollider* csRapidCollideSystem::CreateCollider (iPolygonMesh* mesh)
{
  csRapidCollider* col = new csRapidCollider (mesh);
  return col;
}

bool csRapidCollideSystem::Collide (
  iCollider* collider1, const csReversibleTransform* trans1,
  iCollider* collider2, const csReversibleTransform* trans2)
{
  csRapidCollider* col1 = (csRapidCollider*)collider1;
  csRapidCollider* col2 = (csRapidCollider*)collider2;
  return col1->Collide (*col2, trans1, trans2);
}

int csRapidCollideSystem::CollidePath (
  	iCollider* collider, const csReversibleTransform* trans,
	csVector3& newpos,
	int num_colliders,
	iCollider** colliders,
	csReversibleTransform** transforms)
{
  csRapidCollider* thiscol = (csRapidCollider*)collider;
  return thiscol->CollidePath (trans, newpos,
	num_colliders, colliders, transforms);
}

csCollisionPair* csRapidCollideSystem::GetCollisionPairs ()
{
  return csRapidCollider::GetCollisions ();
}

int csRapidCollideSystem::GetCollisionPairCount ()
{
  return csRapidCollider::numHits;
}

void csRapidCollideSystem::ResetCollisionPairs ()
{
  csRapidCollider::CollideReset ();
  csRapidCollider::numHits = 0;
}


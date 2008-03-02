#ifndef _BASECOLLIDER_H__
#define _BASECOLLIDER_H__

#define CS_MESH_COLLIDER   0
#define CS_TERRAFORMER_COLLIDER  1
#define CS_TERRAIN_COLLIDER 2

#include "src/cmd/collide2/Stdafx.h"

/**
 * A structure used to return collision pairs.
 */
struct csCollisionPair
{
  Opcode::Point a1, b1, c1;	// First triangle
  Opcode::Point a2, b2, c2;	// Second triangle
};

#endif 


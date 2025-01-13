/*
 * CSopcodecollider.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

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
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*                  ported to VS from CS by Ed Sweetman
-------------------------------------------------------------------------
*/

/*
 * Copyright (C) 2020 pyramid3d
 * Copyright (C) 2020-2023 Stephen G. Tuggy, Benjamen R. Meyer
 */

#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODECOL_H
#include "Stdafx.h"
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODECOL_H
#include "csgeom2/opmatrix3.h"
#include "csgeom2/opvector3.h"
#include "csgeom2/opbox.h"
#include "csgeom2/optransfrm.h"
// #include "opcodegarray.h"
#include "basecollider.h"
#include "gfx/mesh.h"

/*
 	How to use Collider.

	The next two calls happen usually once when you first create a unit.

	Create an instance of the collider sending it the appropriate geometry
	csOPCODECollider(const vector<mesh_polygon>&);

	Optionally set if you want to return on first contact or not.
	It defaults to not.
	csOPCODECollider.SetOneHitOnly(bool);

	The rest of the calls occur in your physics loops

	Reset our list of collided pairs of vectors.
	csOPCODECollider.ResetCollisionPairs();

	Check if a collision occurred, sending the other collider and transforms for
	both colliders.   Returns true if we collided.
	csOPCODECollider.Collide(csOPCODECollider&, const csReversibleTransform* first,
	                                            const csReversibleTransform* second);

	If true, retrieve the vectors that collided so we can act upon them.
	csOPCODECollider.GetCollisions();

	We also need the number of collided vectors in case we dont have
	first hit set to true.
	csOPCodeCollider.GetCollisionPairCount();
*/


// Low level collision detection using Opcode library.
class csOPCODECollider {
private:
    /* does what it says.  Takes our mesh_polygon vector and turns it into
    * a linear list of vertexes that we reference in collision trees
    * radius is set in here as well
    */
    void GeometryInitialize(const std::vector<mesh_polygon> &polygons);

    /* callback used to return vertex points when requested from opcode*/
    static void MeshCallback(uint32_t triangle_index,
            Opcode::VertexPointers &triangle, void *user_data);

    /* returns face of mesh where ray collided */
    static void RayCallback(const Opcode::CollisionFace &, void *);

    /* Radius around unit using center of unit and furthest part of unit */
    float radius;

    /* Array of Point's corresponding to vertices of triangles given by mesh_polygon */
    Opcode::Point *vertholder;

    /* OPCODE interfaces. */
    Opcode::Model *m_pCollisionModel;
    Opcode::MeshInterface opcMeshInt;
    Opcode::BVTCache ColCache;
    Opcode::CollisionFace collFace;
    /* Collider type: Tree - Used primarily for mesh on mesh collisions */
    Opcode::AABBTreeCollider TreeCollider;

    /* Collider type: Ray - used to check if a ray collided with the tree collider above */
    Opcode::RayCollider rCollider;

    /* We have to copy our Points to csVector3's because opcode likes Point
    * and VS likes Vector.  */
    void CopyCollisionPairs(csOPCODECollider *col1, csOPCODECollider *col2);

    // std::shared_ptr<VegaStrike::vs_vector<csCollisionPair>> pairs = std::make_shared<VegaStrike::vs_vector<csCollisionPair>>();

public:
    csOPCODECollider(const std::vector<mesh_polygon> &polygons);
    virtual ~csOPCODECollider();

    /* Not used in 0.5 */
    int inline GetColliderType() const {
        return CS_MESH_COLLIDER;
    }

    /* Collides the bolt or beam with this collider, returning true if it occurred */
    bool rayCollide(const Opcode::Ray &boltbeam, Vector &norm, float &distance);

    /* Collides the argument collider with this collider, returning true if it occurred */
    bool Collide(csOPCODECollider &pOtherCollider,
            const csReversibleTransform *pThisTransform = 0,
            const csReversibleTransform *pOtherTransform = 0);

    /* Returns the pair array, as of 0.5 this is a global static var
    * The pair array contains the vertices that have collided as returned
    * by the last collision.   This is concatenated, meaning, if it's not
    * cleared by the client code, the collisions just get pushed onto the
    * array indefinitely.   It should be cleared between collide calls */
    static csCollisionPair *GetCollisions();

    /* clears the pair array */
    static void ResetCollisionPairs();

    /* Returns the size of the pair array */
    static size_t GetCollisionPairCount();

    /* Sets First contact to argument.
    * This means that Collide will return true as soon as the first
    * contact is detected, rather than return the contacts for all
    * detected vertex collisions */
    void SetOneHitOnly(bool fh);

    inline bool GetOneHitOnly() const {
        return (TreeCollider.FirstContactEnabled());
    }

    /* Returns the radius of our collision mesh.  This is the max radius
    * of the mesh we were initialized with */
    inline float GetRadius() const {
        return radius;
    };

    /* Function that returns the Vector given the vertex index
    * Used for displaying the annoying damage particles */
    Vector getVertex(unsigned int which) const;

    /* Returns number of vertexes in model */
    inline unsigned int getNumVertex() const {
        return (m_pCollisionModel->GetMeshInterface()->GetNbVertices());
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODECOL_H

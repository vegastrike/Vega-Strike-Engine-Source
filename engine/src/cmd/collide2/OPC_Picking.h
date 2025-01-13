/*
 * OPC_Picking.h
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to perform "picking".
 *	\file		OPC_Picking.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_PICKING_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_PICKING_H

#ifdef OPC_RAYHIT_CALLBACK

enum CullMode {
    CULLMODE_NONE = 0,
    CULLMODE_CW = 1,
    CULLMODE_CCW = 2
};

typedef CullMode (*CullModeCallback)(uint32_t triangle_index, void *user_data);

OPCODE_API bool SetupAllHits(RayCollider &collider, CollisionFaces &contacts);
OPCODE_API bool SetupClosestHit(RayCollider &collider, CollisionFace &closest_contact);
OPCODE_API bool SetupShadowFeeler(RayCollider &collider);
OPCODE_API bool SetupInOutTest(RayCollider &collider);

OPCODE_API bool Picking(
        CollisionFace &picked_face,
        const Ray &world_ray, const Model &model, const Matrix4x4 *world,
        float min_dist, float max_dist, const Point &view_point, CullModeCallback callback, void *user_data);
#endif

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_PICKING_H

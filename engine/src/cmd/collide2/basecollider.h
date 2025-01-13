/*
 * basecollider.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_BASECOLLIDER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_BASECOLLIDER_H

#define CS_MESH_COLLIDER   0
#define CS_TERRAFORMER_COLLIDER  1
#define CS_TERRAIN_COLLIDER 2

#include "cmd/collide2/Stdafx.h"

/**
 * A structure used to return collision pairs.
 */
struct csCollisionPair {
    csCollisionPair()
            : a1(0.0f, 0.0f, 0.0f),
            b1(0.0f, 0.0f, 0.0f),
            c1(0.0f, 0.0f, 0.0f),
            a2(0.0f, 0.0f, 0.0f),
            b2(0.0f, 0.0f, 0.0f),
            c2(0.0f, 0.0f, 0.0f) {
    }

    Opcode::Point a1, b1, c1;   // First triangle
    Opcode::Point a2, b2, c2;   // Second triangle
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_BASECOLLIDER_H

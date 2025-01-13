/*
 * boltdrawmanager.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_BOLT_DRAW_MANAGER_H
#define VEGA_STRIKE_ENGINE_GFX_BOLT_DRAW_MANAGER_H

#include "gfx/decalqueue.h"
#include "bolt.h"
#include "vec.h"

#include <vector>

class Animation;

class BoltDrawManager {
public:
    class DecalQueue boltdecals;
    static GFXVertexList *boltmesh;
    static QVector camera_position;
    static float pixel_angle;
    static float elapsed_time;

    vector<std::string> animationname;
    vector<Animation *> animations; // Balls are animated
    vector<vector<Bolt> > bolts; // The inner vector is all of the same type.
    vector<vector<Bolt> > balls;

    vector<Bolt> _balls;

    BoltDrawManager();
    ~BoltDrawManager();

    static BoltDrawManager &GetInstance();
    CollideMap::iterator AddBall(const WeaponInfo *typ,
            const Matrix &orientationpos,
            const Vector &shipspeed,
            void *owner,
            CollideMap::iterator hint);

    static void Draw();
};

#endif //VEGA_STRIKE_ENGINE_GFX_BOLT_DRAW_MANAGER_H

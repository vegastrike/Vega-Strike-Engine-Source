/**
 * boltdrawmanager.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "boltdrawmanager.h"

#include "gfx/animation.h"

#include "lin_time.h"
#include "options.h"
#include "universe.h"

QVector BoltDrawManager::camera_position = QVector();
float BoltDrawManager::pixel_angle = 0.0;
float BoltDrawManager::elapsed_time = 0.0;

BoltDrawManager::BoltDrawManager()
{
    if (!boltmesh) {
        GFXVertex vtx[12];
#define V(ii, xx, yy, zz, ss, \
           tt) vtx[ii].x = xx; vtx[ii].y = yy; vtx[ii].z = zz+game_options.bolt_offset+.875; vtx[ii].i = 0; vtx[ii].j = 0; vtx[ii].k = 1; \
    vtx[ii].s = ss; vtx[ii].t = tt;
        V(0, 0, 0, -.875, 0, .5);
        V(1, 0, -1, 0, .875, 1);
        V(2, 0, 0, .125, 1, .5);
        V(3, 0, 1, 0, .875, 0);
        V(4, 0, 0, -.875, 0, .5);
        V(5, -1, 0, 0, .875, 1);
        V(6, 0, 0, .125, 1, .5);
        V(7, 1, 0, 0, .875, 0);
        V(8, 1, 0, 0, .1875, 0);
        V(9, 0, 1, 0, .375, .1875);
        V(10, -1, 0, 0, .1875, .375);
        V(11, 0, -1, 0, 0, .1875);
        boltmesh = new GFXVertexList(GFXQUAD, 12, vtx, 12, false);         //not mutable;
    }
}

GFXVertexList *BoltDrawManager::boltmesh = NULL;

BoltDrawManager::~BoltDrawManager()
{
    unsigned int i;
    for (i = 0; i < animations.size(); i++) {
        delete animations[i];
    }

    for (i = 0; i < balls.size(); i++) {
        for (int j = balls[i].size() - 1; j >= 0; j--) {
            balls[i][j].Destroy(j);
        }
    }

    for (i = 0; i < bolts.size(); i++) {
        for (int j = bolts[i].size() - 1; j >= 0; j--) {
            bolts[i][j].Destroy(j);
        }
    }
}

BoltDrawManager &BoltDrawManager::GetInstance()
{
    static BoltDrawManager instance;    // Guaranteed to be destroyed.
    return instance;                    // Instantiated on first use.
}

void BoltDrawManager::Draw()
{
    GFXDisable(LIGHTING);
    GFXDisable(CULLFACE);
    GFXBlendMode(ONE, game_options.BlendGuns ? ONE : ZERO);
    GFXTextureCoordGenMode(0, NO_GEN, NULL, NULL);
    GFXAlphaTest(GREATER, .1);

    float pixel_angle = 2
            * sin(g_game.fov * M_PI / 180.0
                          / (g_game.y_resolution
                                     > g_game.x_resolution ? g_game.y_resolution : g_game.x_resolution))
            * game_options.bolt_pixel_size;
    pixel_angle *= pixel_angle;
    camera_position = _Universe->AccessCamera()->GetPosition();
    elapsed_time = GetElapsedTime();

    // Iterate over ball types
    Bolt::DrawAllBalls();


    // Iterate over bolt types
    Bolt::DrawAllBolts();

    // Cleanup?
    GFXEnable(LIGHTING);
    GFXEnable(CULLFACE);
    GFXBlendMode(ONE, ZERO);
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXEnable(TEXTURE0);
    GFXColor4f(1, 1, 1, 1);
}

CollideMap::iterator BoltDrawManager::AddBall(const WeaponInfo *typ,
                                              const Matrix &orientationpos,
                                              const Vector &shipspeed,
                                              void *owner,
                                              CollideMap::iterator hint)
{
    Bolt ball = Bolt(typ, orientationpos, shipspeed, owner, hint);             //FIXME turrets won't work! Velocity
    _balls.push_back(ball);
    return _balls[_balls.size() - 1].location;
}

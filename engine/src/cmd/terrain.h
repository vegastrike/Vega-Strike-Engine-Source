/**
 * terrain.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_TERRAIN_H
#define VEGA_STRIKE_ENGINE_CMD_TERRAIN_H

#include "gfx/quadtree.h"
#include "gfx/vec.h"

class Unit;

#define TERRAINRENDER 1
#define TERRAINUPDATE 2
class Terrain : public QuadTree {
public:
    updateparity *updatetransform;
    float TotalSizeX;
    float TotalSizeZ;
    float mass;
    int whichstage;
    char draw;
    void ApplyForce(Unit *un, const Vector &norm, float distance);
public:
    Terrain(const char *filename,
            const Vector &Scales,
            const float mass,
            const float radius,
            updateparity *updatetransform = identityparity);

    void SetTotalSize(float X, float Z) {
        TotalSizeX = X;
        TotalSizeZ = Z;
    }

    ~Terrain();
    void Collide(Unit *un, const Matrix &t);
    void Collide(Unit *un);
    void Collide();
    void SetTransformation(const Matrix &mat);
    static void CollideAll();
    static void DeleteAll();
    void Render();
    static void RenderAll();
    static void UpdateAll(int resolution);
    void DisableDraw();
    void EnableDraw();
    void DisableUpdate();
    void EnableUpdate();
    Vector GetUpVector(const Vector &pos);
};
#endif //VEGA_STRIKE_ENGINE_CMD_TERRAIN_H

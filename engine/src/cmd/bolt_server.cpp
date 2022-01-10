/**
 * bolt_server.cpp
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


#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include <vector>

#include <string>
#include <algorithm>
#include "unit_generic.h"
#include "configxml.h"
GFXVertexList *bolt_draw::boltmesh = NULL;

bolt_draw::~bolt_draw()
{
    unsigned int i;
    for (i = 0; i < balls.size(); i++) {
        for (int j = balls[i].size() - 1; j >= 0; j--) {
            balls[i][j].Destroy(j);
        }
    }
    for (i = 0; i < bolts.size(); i++) {
        for (int j = balls[i].size() - 1; j >= 0; j--) {
            bolts[i][j].Destroy(j);
        }
    }
}

bolt_draw::bolt_draw()
{
    boltmesh = NULL;
    boltdecals = NULL;
}

int Bolt::AddTexture(bolt_draw *q, std::string file)
{
    int decal = 0;
    if (decal >= (int) q->bolts.size()) {
        q->bolts.push_back(vector<Bolt>());
    }
    return decal;
}

int Bolt::AddAnimation(bolt_draw *q, std::string file, QVector cur_position)
{
    int decal = 0;
    if (decal >= (int) q->balls.size()) {
        q->balls.push_back(vector<Bolt>());
    }
    return decal;
}

void Bolt::Draw()
{
}

extern void BoltDestroyGeneric(Bolt *whichbolt, unsigned int index, int decal, bool isBall);

void Bolt::Destroy(unsigned int index)
{
    BoltDestroyGeneric(this, index, decal, type->type != weapon_info::BOLT);
}


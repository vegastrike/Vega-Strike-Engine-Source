/**
 * terrain.cpp
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


#include "cmd/terrain.h"
#include "src/config_xml.h"
#include "root_generic/vs_globals.h"
#include "root_generic/xml_support.h"
#include "src/star_system.h"
#include "cmd/unit_generic.h"
#include "gfx_generic/vec.h"
#include "src/vegastrike.h"
#include "src/universe.h"
#include <vector>
#include "cmd/collection.h"
#include "cmd/building.h"
#include "cmd/damageable.h"

static std::vector<Terrain *> allterrains;

Terrain::Terrain(const char *filename,
        const Vector &scales,
        const float mass,
        const float radius,
        updateparity *updatetransform) : QuadTree(filename, scales, radius),
        TotalSizeX(0),
        TotalSizeZ(0),
        mass(mass),
        whichstage(0) {
    this->updatetransform = updatetransform;
    allterrains.push_back(this);
    draw = TERRAINRENDER | TERRAINUPDATE;
}

Terrain::~Terrain() {
    for (unsigned int i = 0; i < allterrains.size(); i++) {
        if (allterrains[i] == this) {
            allterrains.erase(allterrains.begin() + i);
            break;
        }
    }
}

void Terrain::SetTransformation(const Matrix &Mat) {
    QuadTree::SetTransformation(Mat);
}

void Terrain::ApplyForce(Unit *un, const Vector &normal, float dist) {
    un->ApplyForce(normal * .4 * un->getMass()
            * fabs(normal.Dot((un->GetVelocity() / simulation_atom_var))
                    + fabs(dist) / (simulation_atom_var)));
    Damage damage(.5 * fabs(normal.Dot(un->GetVelocity())) * mass * simulation_atom_var);

    un->ApplyDamage(un->Position().Cast() - normal * un->rSize(),
            -normal,
            damage,
            un, GFXColor(1, 1, 1, 1), NULL);
}

void Terrain::Collide(Unit *un, const Matrix &t) {
    Vector norm;
    if (un->isUnit() == Vega_UnitType::building) {
        return;
    }
    float dist = GetHeight(un->Position().Cast(), norm, t, TotalSizeX, TotalSizeZ) - un->rSize();
    if (dist < 0) {
        ApplyForce(un, norm, -dist);
    }
}

void Terrain::Collide(Unit *un) {
    Collide(un, transformation);
}

void Terrain::DisableUpdate() {
    draw &= (~TERRAINUPDATE);
}

void Terrain::EnableUpdate() {
    draw |= TERRAINUPDATE;
}

void Terrain::DisableDraw() {
    draw &= (~TERRAINRENDER);
}

void Terrain::EnableDraw() {
    draw |= (TERRAINRENDER);
}

void Terrain::Collide() {
    Unit *unit;
    for (un_iter iter = _Universe->activeStarSystem()->getUnitList().createIterator(); (unit = *iter) != NULL; ++iter) {
        Collide(unit);
    }
}

static GFXColor getTerrainColor() {
    float col[4] = {.1f, .1f, .1f, 1.0f};
    return GFXColor(col[0], col[1], col[2], col[3]);
}

void Terrain::CollideAll() {
    for (unsigned int i = 0; i < allterrains.size(); i++) {
        if (allterrains[i]->draw & TERRAINRENDER) {
            allterrains[i]->Collide();
        }
    }
}

void Terrain::DeleteAll() {
    while (!allterrains.empty()) {
        delete allterrains.front();
    }
}

void Terrain::Render() {
    static GFXColor terraincolor(getTerrainColor());
    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(terraincolor);
    QuadTree::Render();
    GFXLightContextAmbient(tmpcol);
}

void Terrain::RenderAll() {
    static GFXColor terraincolor(getTerrainColor());
    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(terraincolor);
    for (unsigned int i = 0; i < allterrains.size(); i++) {
        if (allterrains[i]->draw & TERRAINRENDER) {
            allterrains[i]->Render();
        }
    }
    GFXLightContextAmbient(tmpcol);
}

void Terrain::UpdateAll(int resolution) {
    int res = 4;
    if (resolution == 0) {
        res = 0;
    } else {
        while (resolution > res) {
            res *= 4;
        }
    }
    for (unsigned int i = 0; i < allterrains.size(); i++) {
        if (allterrains[i]->draw & TERRAINUPDATE) {
            allterrains[i]->Update(res, allterrains[i]->whichstage % res, allterrains[i]->updatetransform);
            allterrains[i]->whichstage++;
        }
    }
}

Vector Terrain::GetUpVector(const Vector &pos) {
    return GetNormal(pos, Vector(0, 1, 0));
}


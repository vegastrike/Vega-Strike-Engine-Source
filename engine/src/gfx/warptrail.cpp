/**
 * warptrail.cpp
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


#include "mesh.h"
#include "lin_time.h"
#include "faction_generic.h"
#include "cmd/unit_generic.h"
#include "gfx/camera.h"
#include "config_xml.h"
#include "universe.h"

struct warptrails {
    vector<Mesh *> factions;
    vector<struct WarpTrail *> warps;
    void Draw();
};

warptrails wt;

Mesh *GetWarpMesh(int faction, warptrails *wt);

struct WarpTrail {
    QVector start;
    UnitContainer cur;
    float tim;

    WarpTrail(Unit *un, QVector beg, float tim) : cur(un) {
        start = beg;
        this->tim = tim;
    }

#if 0
    bool Draw( warptrails *w )
    {
        tim -= GetElapsedTime();
        Unit   *un = cur.GetUnit();
        if (!un) return false;
        Mesh   *m  = GetWarpMesh( un->faction, w );
        if (!m)
            return false;
        QVector end( un->Position() );
        float   length = (end-start).Magnitude();
        float   d = ( .5*(end+start)-_Universe->AccessCamera()->GetPosition().Cast() ).Magnitude();
        Vector  p, q, r;
        r = (end-start).Cast();
        q = Vector( 0, 1, 0 );
        p = r.Cross( q );
        q = r.Cross( p );
        p.Normalize();
        q.Normalize();
        Matrix matrix( p, q, r, end );
        m->Draw( length, matrix, d );
        return tim > 0;
    }
#else

    bool Draw(warptrails *w) {
        tim -= GetElapsedTime();
        Unit *un = cur.GetUnit();
        if (!un) {
            return false;
        }
        Mesh *m = GetWarpMesh(un->faction, w);
        if (!m) {
            return false;
        }
        QVector end(un->Position());
        float length = (end - start).Magnitude();
        float d = (end - _Universe->AccessCamera()->GetPosition().Cast()).Magnitude();
        Vector p, q, r;
        r = (end - start).Cast();
        r.Normalize();
        static float stretch = XMLSupport::parse_float(vs_config->getVariable("graphics", "warp_trail_stretch", "300"));
        r *= un->rSize() * stretch;
        q = Vector(0, 1, 0);
        p = r.Cross(q);
        q = p.Cross(r);
        p.Normalize();
        p *= un->rSize();
        q.Normalize();
        q *= un->rSize();
        Matrix matrix(p, q, r, end);
        m->Draw(length, matrix, d);
        d = (start - _Universe->AccessCamera()->GetPosition().Cast()).Magnitude();
        matrix.p = start + r;
        m->Draw(length, matrix, d);
        return tim > 0;
    }

#endif
};

void warptrails::Draw() {
    for (unsigned int i = 0; i < warps.size(); ++i) {
        if (!warps[i]->Draw(this)) {
            delete warps[i];
            warps.erase(warps.begin() + i);
            i--;
        }
    }
}

void AddWarp(Unit *un, QVector beg, float tim) {
    wt.warps.push_back(new WarpTrail(un, beg, tim));
}

void WarpTrailDraw() {
    wt.Draw();
}

Mesh *GetWarpMesh(int faction, warptrails *w) {
    using namespace VSFileSystem;
    while (faction >= static_cast<int>(w->factions.size())) {
        w->factions.push_back(NULL);
    }
    string fac = FactionUtil::GetFaction(faction);
    fac += "_warp.bfxm";
    VSError err;
    if ((err = LookForFile(fac, MeshFile)) > Ok) {
        fac = "neutral_warp.bfxm";
        if ((err = LookForFile(fac, MeshFile)) > Ok) {
            return NULL;
        }
    }
    if (!w->factions[faction]) {
        w->factions[faction] = Mesh::LoadMesh(fac.c_str(), Vector(1, 1, 1), faction, NULL);
    }
    return w->factions[faction];
}


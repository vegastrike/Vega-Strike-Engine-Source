/*
 * warptrail.cpp
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "mesh.h"
#include "lin_time.h"
#include "faction_generic.h"
#include "cmd/unit_generic.h"
#include "gfx/camera.h"
#include "config_xml.h"
#include "universe.h"

struct WarpTrail;

struct warptrails : vega_types::EnableSharedFromThis<warptrails> {
    vega_types::SequenceContainer<vega_types::SharedPtr<Mesh>> factions;
    vega_types::SequenceContainer<vega_types::SharedPtr<WarpTrail>> warps;
    void Draw();
};

warptrails wt;

vega_types::SharedPtr<Mesh> GetWarpMesh(int faction, vega_types::SharedPtr<warptrails> w);

struct WarpTrail : vega_types::EnableSharedFromThis<WarpTrail> {
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

    bool Draw(vega_types::SharedPtr<warptrails> w) {
        tim -= GetElapsedTime();
        Unit *un = cur.GetUnit();
        if (!un) {
            return false;
        }
        vega_types::SharedPtr<Mesh> m = GetWarpMesh(un->faction, w);
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
        if (!warps[i]->Draw(shared_from_this())) {
            warps[i].reset();
            warps.erase(warps.begin() + i);
            i--;
        }
    }
}

void AddWarp(Unit *un, QVector beg, float tim) {
    wt.warps.push_back(vega_types::MakeShared<WarpTrail>(un, beg, tim));
}

void WarpTrailDraw() {
    wt.Draw();
}

vega_types::SharedPtr<Mesh> GetWarpMesh(int faction, vega_types::SharedPtr<warptrails> w) {
    using namespace VSFileSystem;
    while (faction >= static_cast<int>(w->factions.size())) {
        w->factions.push_back(nullptr);
    }
    string fac = FactionUtil::GetFaction(faction);
    fac += "_warp.bfxm";
    if ((LookForFile(fac, MeshFile)) > Ok) {
        fac = "neutral_warp.bfxm";
        if ((LookForFile(fac, MeshFile)) > Ok) {
            return nullptr;
        }
    }
    if (!w->factions[faction]) {
        w->factions[faction] = Mesh::LoadMesh(fac.c_str(), Vector(1, 1, 1), faction, nullptr);
    }
    return w->factions[faction];
}


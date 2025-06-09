/*
 * mesh_fx.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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


#include "gfx_generic/mesh.h"
#include "root_generic/lin_time.h"
#define MAXLOCALFX (6)
static float startpotency = 20;
static float endpotency = 4;
static float flickertime = 3.5;
const float mindamage = .1;

static void AvLights(float target[4], const float other[4]) {
    target[0] = .5 * (target[0] + other[0]);
    target[1] = .5 * (target[1] + other[1]);
    target[2] = .5 * (target[2] + other[2]);
    target[3] = .5 * (target[3] + other[3]);
}

MeshFX::MeshFX(const float TTL,
        const float delta,
        const bool enabled,
        const GFXColor &vect,
        const GFXColor &diffuse,
        const GFXColor &specular,
        const GFXColor &ambient,
        const GFXColor &attenuate) :
        GFXLight(enabled, vect, diffuse, specular, ambient, attenuate) {
    this->TTL = TTL;
    this->TTD = this->TTL;
    this->delta = delta;
}

void MeshFX::MergeLights(const MeshFX &other) {
    //if (TTL>0) {
    delta = .5 * (other.delta + this->delta);
    /*  TTL = (TTL>other.TTL)
     *   ?
     *   (.666667*TTL+.33333*other.TTL)
     *   :
     *   (.666667*other.TTL+.333333*TTL);*/
    TTL = .5 * (TTL + other.TTL);
    TTD = .5 * (TTD + other.TTD);
    Vector vec(vect[0], vect[1], vect[2]);
    vec *= .5;
    Vector othervec(other.vect[0], other.vect[1], other.vect[2]);
    othervec *= .5;
    //float distsqr = ((vec-othervec)).Dot ((vec-othervec));
    options |= other.options;
    vec = vec + othervec;
    vect[0] = vec.i;
    vect[1] = vec.j;
    vect[2] = vec.k;
    AvLights(diffuse, other.diffuse);
    AvLights(specular, other.specular);
    AvLights(ambient, other.ambient);
    /*    attenuate[2]=1./attenuate[2];
    *  attenuate[2]+=1./other.attenuate[2]+distsqr;
    *  attenuate[2]= 1./attenuate[2];
    *
    *  attenuate[1]=1./attenuate[1];
    *  attenuate[1]+=1./other.attenuate[1]+sqrtf(distsqr);
    *  attenuate[1]= 1./attenuate[1];*/
    //} else {
    //memcpy(this, &other, sizeof (MeshFX));
    //}
}

bool MeshFX::Update(float howmuchtime) {
    TTL -= howmuchtime;
    if (TTL < 0) {
        TTL = 0;
        TTD -= howmuchtime;
        attenuate[2] += 1.5 * delta * howmuchtime;
        //attenuate[1]+=2*delta*GetElapsedTime();

        //attenuate[2]*=1+2*delta*GetElapsedTime();
        //attenuate[1]*=1+2*delta*GetElapsedTime();
    } else {
        attenuate[2] -= delta * howmuchtime;
        if (attenuate[2] < delta) {
            attenuate[2] = delta / 4;
        }
        //attenuate[1]-=1.25*delta*GetElapsedTime();
        //attenuate[2]*=1- .5*delta*GetElapsedTime();
        //attenuate[1]*=1- .5*delta*GetElapsedTime();
    }
    return TTD > 0;
}

void Mesh::AddDamageFX(const Vector &pnt, const Vector &norm, const float damage, const GFXColor &col) {
    float r_size = rSize();

    if (r_size == 0) {
        return;
    }

    Vector loc(pnt + norm);
    /*if (!(norm.i||norm.j||norm.k)) */ {
        loc = pnt;
        loc.Normalize();
        loc *= (1 + rSize());
    }

    GFXColor tmp(col.r, col.g, col.b, col.a);
    float numsec = flickertime * ((damage < mindamage) ? mindamage : damage);
    MeshFX newFX(numsec, (startpotency - endpotency) / (numsec * rSize() * rSize()), true,
            GFXColor(loc.i, loc.j, loc.k, 1),
            tmp,
            GFXColor(0, 0, 0, 1),
            tmp,
            GFXColor(1, 0, startpotency / (r_size * r_size)));
    newFX.setSize(rSize());
    if (LocalFX.size() >= MAXLOCALFX) {
        LocalFX[(rand() % (LocalFX.size()))].MergeLights(newFX);
    } else {
        LocalFX.push_back(newFX);
    }
}

void Mesh::UpdateFX(float howmuchtime) {
    //adjusts lights by TTL, eventually removing them
    for (int i = LocalFX.size() - 1; i >= 0; i--) {
        if (!LocalFX[i].Update(howmuchtime)) {
            LocalFX.erase(LocalFX.begin() + i);
        }
    }
}

void Mesh::EnableSpecialFX() {
    draw_sequence = MESH_SPECIAL_FX_ONLY;
    setEnvMap(GFXFALSE);
    setLighting(GFXTRUE);
    if (orig) {
        orig->draw_sequence = MESH_SPECIAL_FX_ONLY;
        orig->blendSrc = orig->blendDst = ONE;
        orig->setEnvMap(GFXFALSE);
        orig->setLighting(GFXTRUE);
    }
    blendSrc = ONE;
    blendDst = ONE;
}


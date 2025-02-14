/*
 * atmosphere.cpp
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


#include "cmd/atmosphere.h"
#include "gfx/mesh.h"
#include "gfx/matrix.h"
#include "gfx/vec.h"
#include "gfxlib_struct.h"
#include "gfx/sphere.h"
#include "cmd/planet.h"
#include "star_system.h"
#include "cmd/collection.h"
#include "cmd/unit_generic.h"
#include "universe.h"

Atmosphere::SunBox::~SunBox() {
    if (sunbox != nullptr) {
        delete sunbox;
        sunbox = nullptr;
    }
}

void Atmosphere::setArray(float c0[4], const GFXColor &c1) {
    c0[0] = c1.r;
    c0[1] = c1.g;
    c0[2] = c1.b;
    c0[3] = c1.a;
}

void Atmosphere::setArray1(float c0[3], const GFXColor &c1) {
    c0[0] = c1.r;
    c0[1] = c1.g;
    c0[2] = c1.b;
}

Atmosphere::Atmosphere(const Parameters &params) : user_params(params), divisions(64) {
    dome = new SphereMesh(params.radius,
            divisions,
            divisions,
            "white.bmp",
            "",
            NULL,
            true,
            ONE,
            ZERO,
            false,
            0,
            M_PI / 2);
}

Atmosphere::~Atmosphere() {
    for (size_t a = 0; a < sunboxes.size(); ++a) {
        delete sunboxes[a];
    }
}

const Atmosphere::Parameters &Atmosphere::parameters() {
    return user_params;
}

void Atmosphere::SetParameters(const Parameters &params) {
    user_params = params;
}

void Atmosphere::Update(const QVector &position, const Matrix &tmatrix) {
    Planet *currPlanet;
    StarSystem *system = _Universe->activeStarSystem();
    for (size_t a = 0; a < sunboxes.size(); ++a) {
        delete sunboxes[a];
    }
    sunboxes.clear();
    QVector localDir;
    float rho1 = 0.0;
    Unit *primary;
    for (un_iter iter = system->getUnitList().createIterator(); (primary = *iter) != NULL; ++iter) {
        if (primary->isUnit() == Vega_UnitType::planet && (currPlanet = (Planet *) primary)->hasLights()) {
            /* for now just assume all planets with lights are really bright */
            QVector direction = (currPlanet->Position() - position);
            direction.Normalize();
            double rho = direction * InvTransformNormal(tmatrix, QVector(0, 1, 0));
            if (rho > 0) {
                /* above the horizon */
                QVector localDirection = InvTransformNormal(tmatrix, direction);

                /* bad */
                localDir = localDirection;
                rho1 = rho;

                /* need a function for the sunbox size. for now, say it takes up a quarter
                 *  of the screen */
                /* drop the z value and find the theta */
                QVector lprime = localDirection;
                lprime.k = 0;
                lprime.Normalize();
                //float theta = atan2(lprime.i,lprime.j);
                //float size = .125;
                sunboxes.push_back(new SunBox(NULL));
                break;
            }
        }
    }
    if (!sunboxes.empty()) {
        float rho = acos(rho1) / (PI / 2);
        float radius = user_params.radius;
        /* index 0 is the top color, index 1 is the bottom color */
        GFXLight light0 = GFXLight();
        light0.SetProperties(AMBIENT,
                rho * user_params.high_ambient_color[0] + (1 - rho) * user_params.low_ambient_color[0]);
        light0.SetProperties(DIFFUSE, rho * user_params.high_color[0] + (1 - rho) * user_params.low_color[0]);
        light0.SetProperties(ATTENUATE, 0.5 * GFXColor(1, 0.25 / radius, 0));
        light0.SetProperties(POSITION, GFXColor(0, 1.1 * radius, 0, 1));

        /* do a linear interpolation between this and the next one */

        GFXLight light1 = GFXLight();
        light1.SetProperties(AMBIENT,
                (1 - rho) * user_params.high_ambient_color[1] + rho * user_params.low_ambient_color[1]);
        light1.SetProperties(DIFFUSE, (1 - rho) * user_params.high_color[1] + rho * user_params.low_color[1]);
        light1.SetProperties(ATTENUATE, 0.5 * GFXColor(1, 0.75 / radius, 0));
        light1.SetProperties(POSITION, GFXColor(0, -1.1 * radius, 0, 1));

        /* Note!! make sure that this light never goes too far around the sphere */
        GFXLight light2 = light1;         /* -80 degree declination from sun position */
        Matrix m;
        QVector r;
        ScaledCrossProduct(QVector(0, 1, 0), localDir, r);
        Rotate(m, r.Cast(), -80 * (PI / 180));
        r = Transform(m, QVector(0, 0, 1));
        float sradius = 1.1 * radius;
        light2.SetProperties(POSITION, GFXColor(sradius * r.i, sradius * r.j, sradius * r.k, 1));

        GFXCreateLight(l0, light0, true);
        GFXCreateLight(l1, light1, true);
        GFXEnableLight(l0);
        GFXEnableLight(l1);
    }
}

static std::vector<Atmosphere *> draw_queue;

void Atmosphere::SetMatricesAndDraw(const QVector &pos, const Matrix mat) {
    CopyMatrix(tmatrix, mat);
    position = pos;
    draw_queue.push_back(this);
}

void Atmosphere::ProcessDrawQueue() {
    GFXEnable(LIGHTING);
    GFXDisable(TEXTURE1);
    GFXDisable(TEXTURE0);
    GFXDisable(DEPTHWRITE);
    while (!draw_queue.empty()) {
        draw_queue.back()->Draw();
        draw_queue.pop_back();
    }
}

void Atmosphere::Draw() {
    GFXDisable(TEXTURE1);
    Matrix rot(1, 0, 0,
            0, 0, -1,
            0, 1, 0,
            QVector(0, 0, 0));
    Matrix rot1;
    MultMatrix(rot1, tmatrix, rot);
    CopyMatrix(rot1, tmatrix);

    Vector tmp(rot1.getR());
    Vector tmp2(rot1.getQ());

    rot1.r[6] = -tmp.i;
    rot1.r[7] = -tmp.j;
    rot1.r[8] = -tmp.k;

    rot1.r[3] = -tmp2.i;
    rot1.r[4] = -tmp2.j;
    rot1.r[5] = -tmp2.k;

    GFXMaterial a = {
            0, 0, 0, 0,
            1, 1, 1, 1,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0
    };
    dome->SetMaterial(a);
    GFXLoadMatrixModel(rot1);
    Update(position, rot1);

    GFXDisable(DEPTHWRITE);
    dome->DrawNow(100000, GFXFALSE, rot1);
    GFXDisableLight(l0);
    GFXDisableLight(l1);
    GFXDeleteLight(l0);
    GFXDeleteLight(l1);
}

void Atmosphere::DrawAtmospheres() {
    abort();
}


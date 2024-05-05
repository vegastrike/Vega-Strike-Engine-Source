/*
 * halo_system.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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


#include <vector>
#include <string>
#include "vec.h"
#include "matrix.h"
#include "cmd/unit_generic.h"
#include "halo_system.h"
#include "universe.h"
#include <stdlib.h>
#include <stdio.h>
#include "vegastrike.h"
#include "mesh.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "gfx/particle.h"
#include "lin_time.h"
#include "animation.h"
#include "car_assist.h"
#include "cmd/collide2/CSopcodecollider.h"
#include "options.h"
#include "cmd/unit_collide.h"

#define HALO_SMOOTHING_UP_FACTOR (0.02)
#define HALO_SMOOTHING_DOWN_FACTOR (0.01)
#define HALO_STEERING_UP_FACTOR (0.00)
#define HALO_STEERING_DOWN_FACTOR (0.01)
#define HALO_STABILIZATION_RANGE (0.25)

static float mymin(float a, float b) {
    return a > b ? b : a;
}

static float mymax(float a, float b) {
    return a > b ? a : b;
}

void DoParticles(QVector pos,
        float percent,
        const Vector &basevelocity,
        const Vector &velocity,
        float radial_size,
        float particle_size,
        int faction) {
    static ParticleEmitter sparkles(&particleTrail, "sparkle");

    const float *col = FactionUtil::GetSparkColor(faction);

    sparkles.doParticles(
            pos, radial_size, percent, basevelocity, velocity, particle_size,
            GFXColor(col[0], col[1], col[2], 1.0f)
    );
}

void LaunchOneParticle(const Matrix &mat, const Vector &vel, unsigned int seed, Unit *mush, float hull, int faction) {
    if (mush) {
        bool done = false;
        Vector back = vel;
        back.Normalize();
        back *= -vs_options::instance().sparkleabsolutespeed;

        collideTrees *colTrees = mush->colTrees;
        if (colTrees) {
            if (colTrees->usingColTree()) {
                csOPCODECollider *colTree = colTrees->rapidColliders[0];
                unsigned int numvert = colTree->getNumVertex();
                if (numvert) {
                    unsigned int whichvert = seed % numvert;
                    QVector v(colTree->getVertex(whichvert).Cast());
                    v = Transform(mat, v);
                    DoParticles(v,
                            hull,
                            vel,
                            back,
                            0,
                            mush->rSize() * vs_options::instance().sparkleenginesizerelativetoship,
                            faction);
                    done = true;
                }
            }
        }
        if (!done) {
            // maybe ray collision?
        }
        if (!done) {
            unsigned int siz = (unsigned int) (2 * mush->rSize());
            if (siz != 0) {
                QVector v((seed % siz) - siz / 2,
                        (seed % siz) - siz / 2,
                        (seed % siz) - siz / 2);
                DoParticles(v,
                        hull,
                        vel,
                        back,
                        0,
                        mush->rSize() * vs_options::instance().sparkleenginesizerelativetoship,
                        faction);
                done = true;
            }
        }
    }
}

HaloSystem::HaloSystem() {
    VSCONSTRUCT2('h')
}

unsigned int HaloSystem::AddHalo(const char *filename,
        const Matrix &trans,
        const Vector &size,
        const GFXColor &col,
        std::string type,
        float activation_accel) {
    int neutralfac = FactionUtil::GetNeutralFaction();
    halo.push_back(Halo());
    halo.back().trans = trans;
    halo.back().size = Vector(size.i * vs_options::instance().engine_radii_scale,
            size.j * vs_options::instance().engine_radii_scale,
            size.k * vs_options::instance().engine_length_scale);
    halo.back().mesh = Mesh::LoadMesh((string(filename)).c_str(), Vector(1, 1, 1), neutralfac, NULL);
    halo.back().activation = activation_accel * vs_options::instance().game_speed;
    halo.back().oscale = 0;
    halo.back().sparkle_accum = 0;
    halo.back().sparkle_rate = 0.5 + rand() * 0.5 / float(RAND_MAX);
    return halo.size() - 1;
}

static float HaloAccelSmooth(float linaccel, float olinaccel, float maxlinaccel) {
    linaccel = mymax(0, mymin(maxlinaccel, linaccel));     //Clamp input, somehow, sometimes it's not clamped
    float phase =
            pow(((linaccel > olinaccel) ? HALO_SMOOTHING_UP_FACTOR : HALO_SMOOTHING_DOWN_FACTOR), GetElapsedTime());
    float olinaccel2;
    if (linaccel > olinaccel) {
        olinaccel2 = mymin(linaccel, olinaccel + maxlinaccel * HALO_STEERING_UP_FACTOR);
    } else {
        olinaccel2 = mymax(linaccel, olinaccel - maxlinaccel * HALO_STEERING_DOWN_FACTOR);
    }
    linaccel = (1 - phase) * linaccel + phase * olinaccel2;
    linaccel = mymax(0, mymin(maxlinaccel, linaccel));
    return linaccel;
}

void HaloSystem::Draw(const Matrix &trans,
        const Vector &scale,
        int halo_alpha,
        float nebdist,
        float hullpercent,
        const Vector &velocity,
        const Vector &linaccel,
        const Vector &angaccel,
        float maxaccel,
        float maxvelocity,
        int faction) {
    if (halo_alpha >= 0) {
        halo_alpha /= 2;
        halo_alpha |= 1;
    }
    if (maxaccel <= 0) {
        maxaccel = 1;
    }
    if (maxvelocity <= 0) {
        maxvelocity = 1;
    }

    double sparkledelta = GetElapsedTime() * vs_options::instance().halosparklerate;

    for (std::vector<Halo>::iterator i = halo.begin(); i != halo.end(); ++i) {
        Vector thrustvector = TransformNormal(trans, i->trans.getR()).Normalize();
        float value, maxvalue, minvalue;
        if (vs_options::instance().halos_by_velocity) {
            value = velocity.Dot(thrustvector);
            maxvalue = sqrt(maxvelocity);
            minvalue = i->activation;
        } else {
            Vector relpos = TransformNormal(trans, i->trans.p);
            Vector accel = linaccel + relpos.Cross(angaccel);
            float accelmag = accel.Dot(thrustvector);
            i->oscale = HaloAccelSmooth(accelmag / maxaccel, i->oscale, 1.0f);
            value = i->oscale;
            maxvalue = 1.0f;
            minvalue = i->activation / maxaccel;
        }
        if ((value > minvalue) && (scale.k > 0)) {
            Matrix m = trans * i->trans;
            ScaleMatrix(m, Vector(scale.i * i->size.i, scale.j * i->size.j, scale.k * i->size.k * value / maxvalue));

            float maxfade =
                    minvalue * (1.0 - vs_options::instance().percent_halo_fade_in) + maxvalue * vs_options::instance().percent_halo_fade_in;
            int alpha = halo_alpha;
            if (value < maxfade) {
                if (alpha < 0) {
                    alpha = CLKSCALE;
                }
                alpha = int(alpha * (value - minvalue) / (maxfade - minvalue));
                alpha |= 1;
            }

            GFXColor blend = GFXColor(vs_options::instance().engine_color_red,
                    vs_options::instance().engine_color_green,
                    vs_options::instance().engine_color_blue,
                    1);
            if (value > maxvalue * vs_options::instance().percent_afterburner_color_change) {
                float test = value - maxvalue * vs_options::instance().percent_afterburner_color_change;
                test /= maxvalue * vs_options::instance().percent_afterburner_color_change;
                if (!(test < 1.0)) {
                    test = 1.0;
                }
                float r = vs_options::instance().afterburner_color_red * test + vs_options::instance().engine_color_red * (1.0 - test);
                float g = vs_options::instance().afterburner_color_green * test + vs_options::instance().engine_color_green * (1.0 - test);
                float b = vs_options::instance().afterburner_color_blue * test + vs_options::instance().engine_color_blue * (1.0 - test);
                blend = GFXColor(r, g, b, 1.0);
            }

            MeshFX xtraFX = MeshFX(1.0, 1.0,
                    true,
                    GFXColor(1, 1, 1, 1),
                    GFXColor(1, 1, 1, 1),
                    GFXColor(1, 1, 1, 1),
                    blend);

            static Cloak dummy_cloak;
            i->mesh->Draw(50000000000000.0, m, alpha, dummy_cloak, nebdist, 0, false, &xtraFX);

            // If damaged, and halo is back-facing
            if (hullpercent < .99 && ((i->trans.getR().z / i->trans.getR().Magnitude()) > 0.707)) {
                float vpercent = value / maxvalue;
                i->sparkle_accum += sparkledelta * i->sparkle_rate * vpercent;
                int spawn = (int) (i->sparkle_accum);

                if (spawn > 0) {
                    i->sparkle_accum -= 1;

                    float rsize = i->mesh->rSize() * scale.i * i->size.i;
                    Vector pvelocity = thrustvector * -rsize * vs_options::instance().halosparklespeed * vpercent;

                    DoParticles(m.p,
                            hullpercent,
                            velocity,
                            pvelocity,
                            rsize,
                            rsize * vs_options::instance().halosparklescale,
                            faction);
                }
            } else {
                i->sparkle_accum = 0;
            }
        } else {
            i->sparkle_accum = 0;
        }
    }
}

HaloSystem::~HaloSystem() {
    VSDESTRUCT2
    for (std::vector<Halo>::iterator i = halo.begin(); i != halo.end(); ++i) {
        if (i->mesh != nullptr) {
            delete i->mesh;
            i->mesh = nullptr;
        }
    }
}

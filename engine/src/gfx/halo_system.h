/*
 * halo_system.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_HALOSYSTEM_H
#define VEGA_STRIKE_ENGINE_GFX_HALOSYSTEM_H

#include <vector>
#include <string>

#include "gfx_generic/matrix.h"
class Mesh;
struct GFXColor;
class Unit;

void DoParticles(QVector pos, float hullpercent, const Vector &velocity, float radial_size, float hullpct, int faction);
void LaunchOneParticle(const Matrix &mat, const Vector &vel, unsigned int seed, Unit *mesh, float hullpct, int faction);

class HaloSystem {
private:
    struct Halo {
        Matrix trans;
        Vector size;
        Mesh *mesh;
        double sparkle_accum;
        float sparkle_rate;
        float activation;
        float oscale;
    };
    std::vector<Halo> halo;

public:
    virtual ~HaloSystem();
    HaloSystem();

    unsigned int NumHalos() const {
        return halo.size();
    }

    unsigned int AddHalo(const char *filename,
            const Matrix &trans,
            const Vector &size,
            const GFXColor &col,
            std::string halo_type /*when it grows*/,
            float activation_accel);
//Draw draws smoke and damage fx:
    void Draw(const Matrix &trans,
            const Vector &scale,
            int halo_alpha,
            float nebdist,
            float hullpercentage,
            const Vector &velocity,
            const Vector &linaccel,
            const Vector &angaccel,
            float maxaccel,
            float maxvelocity,
            int faction);
};

#endif //VEGA_STRIKE_ENGINE_GFX_HALOSYSTEM_H

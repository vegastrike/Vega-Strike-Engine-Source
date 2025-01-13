/*
 * vertex.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GFX_VERTEX_H
#define VEGA_STRIKE_ENGINE_GFX_VERTEX_H

struct glVertex {
    float s;
    float t;
    float i;
    float j;
    float k;
    float x;
    float y;
    float z;

    glVertex &SetTexCoord(float s, float t) {
        this->s = s;
        this->t = t;
        return *this;
    }

    glVertex &SetNormal(const Vector &norm) {
        i = norm.i;
        j = norm.j;
        k = norm.k;
        return *this;
    }

    glVertex &SetVertex(const Vector &vert) {
        x = vert.i;
        y = vert.j;
        z = vert.k;
        return *this;
    }
};

#endif //VEGA_STRIKE_ENGINE_GFX_VERTEX_H

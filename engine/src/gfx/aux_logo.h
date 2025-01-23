/*
 * aux_logo.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_LOGO_H
#define VEGA_STRIKE_ENGINE_GFX_LOGO_H

#include "matrix.h"
#include "gfxlib.h"
#include "hashtable.h"
#include "vec.h"

#include <vector>

//struct glVertex;
class Mesh;

class Texture;

class Logo {
    int numlogos;
//glVertex **LogoCorner;
    GFXVertexList *vlist;
    Texture *Decal;
    static Hashtable<int, Logo, 257> decalHash;

    Logo() {
    }

protected:
    friend class Mesh;

    int refcount;   //number of references to draw_queue
    Logo *owner_of_draw_queue;     //owner of the draw_queue
    vector<DrawContext> *draw_queue;
    bool will_be_drawn;

    void ProcessDrawQueue();

public:
    Logo(int numberlogos,
            Vector *center,
            Vector *normal,
            float *sizes,
            float *rotations,
            float offset,
            Texture *Dec,
            Vector *Ref);

    Logo(const Logo &rval) {
        *this = rval;
    }

    ~Logo();

    void SetDecal(Texture *decal);

    void Draw(const Matrix &m);
};

#endif //VEGA_STRIKE_ENGINE_GFX_LOGO_H

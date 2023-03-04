/*
 * Copyright (C) 2001-2022 Daniel Horn, ace123, surfdargent, klaussfreire,
 * jacks, dan_w, ashieh, griwodz, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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


#ifndef LOGO_H_
#define LOGO_H_

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
    vega_types::SharedPtr<Texture> Decal;
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
         vega_types::SharedPtr<Texture> Dec,
         Vector *Ref);

    Logo(const Logo &rval) {
        *this = rval;
    }

    ~Logo();

    void SetDecal(vega_types::SharedPtr<Texture> decal);

    void Draw(const Matrix &m);
};

#endif


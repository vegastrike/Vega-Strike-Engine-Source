/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h> //for mkdir
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "gfx/sprite.h"
#include "objconv/basemaker/base_maker_texture.h"

#ifndef M_PI_2
# define M_PI_2 (1.57079632679489661923)
#endif

static float *mview = NULL;
static int bogus_int; //added by chuck_starchaser to squash ignored returns warnings.

VSSprite::VSSprite(const char *file, enum FILTER texturefilter, GFXBOOL force) {
    xcenter = 0;
    ycenter = 0;
    widtho2 = 0;
    heighto2 = 0;
    rotation = 0;
    surface = NULL;
    maxs = maxt = 0;
    bogus_int = chdir("sprites");
    FILE *f = nullptr;
    if (file[0] != '\0') {
        f = fopen(file, "rt");
    }
    if (f) {
        char texture[64] = {0};
        char texturea[64] = {0};
        bogus_int = fscanf(f, "%63s %63s", texture, texturea);
        bogus_int = fscanf(f, "%f %f", &widtho2, &heighto2);
        bogus_int = fscanf(f, "%f %f", &xcenter, &ycenter);

        widtho2 /= 2;
        heighto2 /= -2;
        surface = NULL;
        if (texturea[0] == '0') {
            surface = new Texture(texture);
        } else {
            surface = new Texture(texture);
        }
        if (!surface) {
            delete surface;
            surface = NULL;
        }
        //Finally close file
        fclose(f);
    } else {
        widtho2 = heighto2 = 0;
        xcenter = ycenter = 0;
    }
    bogus_int = chdir("..");
}

using std::cout;
using std::cerr;
using std::endl;

void VSSprite::ReadTexture(FILE *f) {
    if (!f) {
        widtho2 = heighto2 = 0;
        xcenter = ycenter = 0;
        cerr << "VSSprite::ReadTexture error : VSFile not valid" << endl;
        return;
    }
    surface = new Texture(f);
}

VSSprite::~VSSprite() {
    if (surface != nullptr) {
        delete surface;
        surface = nullptr;
    }
}

void VSSprite::SetST(const float s, const float t) {
    maxs = s;
    maxt = t;
}

void VSSprite::DrawHere(Vector &ll, Vector &lr, Vector &ur, Vector &ul) {
    if (rotation) {
        const float cw = widtho2 * cos(rotation);
        const float sw = widtho2 * sin(rotation);
        const float ch = heighto2 * cos(M_PI_2 + rotation);
        const float sh = heighto2 * sin(M_PI_2 + rotation);
        const float wnew = cw + ch;
        const float hnew = sw + sh;
        ll = Vector(xcenter - wnew, ycenter + hnew, 0.00f);
        lr = Vector(xcenter + wnew, ycenter + hnew, 0.00f);
        ur = Vector(xcenter + wnew, ycenter - hnew, 0.00f);
        ul = Vector(xcenter - wnew, ycenter - hnew, 0.00f);
    } else {
        ll = Vector(xcenter - widtho2, ycenter + heighto2, 0.00f);
        lr = Vector(xcenter + widtho2, ycenter + heighto2, 0.00f);
        ur = Vector(xcenter + widtho2, ycenter - heighto2, 0.00f);
        ul = Vector(xcenter - widtho2, ycenter - heighto2, 0.00f);
    }
}

void VSSprite::Draw() {
    if (surface) {
        //don't do anything if no surface
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        surface->bind();
        glBegin(GL_QUADS);
        Vector ll, lr, ur, ul;
        DrawHere(ll, lr, ur, ul);
        glTexCoord2f(maxs, 1);
        glVertex3f(ll.i, ll.j, ll.k);
        glTexCoord2f(1, 1);
        glVertex3f(lr.i, lr.j, lr.k);
        glTexCoord2f(1, maxt);
        glVertex3f(ur.i, ur.j, ur.k);
        glTexCoord2f(maxs, maxt);
        glVertex3f(ul.i, ul.j, ul.k);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
    }
}

void VSSprite::SetPosition(const float &x1, const float &y1) {
    xcenter = x1;
    ycenter = y1;
}

void VSSprite::GetPosition(float &x1, float &y1) {
    x1 = xcenter;
    y1 = ycenter;
}

void VSSprite::SetSize(float x1, float y1) {
    widtho2 = x1 / 2;
    heighto2 = y1 / 2;
}

void VSSprite::GetSize(float &x1, float &y1) {
    x1 = widtho2 * 2;
    y1 = heighto2 * 2;
}

void VSSprite::SetRotation(const float &rot) {
    rotation = rot;
}

void VSSprite::GetRotation(float &rot) {
    rot = rotation;
}

/*
void VSExit( int code )
{
    exit( code );
}
*/

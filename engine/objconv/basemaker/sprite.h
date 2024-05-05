/*
 * sprite.h
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_OBJCONV_SPRITE_H
#define VEGA_STRIKE_ENGINE_OBJCONV_SPRITE_H

#include "gfx/quaternion.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "gfx/hud.h"
#include "vs_globals.h"

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#include <glut.h>
#include <glext.h>

#include <iostream>
//#include <stdlib>

class Texture;

class VSSprite {
    float xcenter;
    float ycenter;
    float widtho2;
    float heighto2;
    float maxs, maxt;
    float rotation;
    Texture *surface;
public:
    VSSprite(const char *file, enum FILTER texturefilter = BILINEAR, GFXBOOL force = GFXFALSE);
    ~VSSprite();

    bool LoadSuccess() {
        return surface != NULL;
    }

    void Draw();
    void DrawHere(Vector &ll, Vector &lr, Vector &ur, Vector &ul);

    void Rotate(const float &rad) {
        rotation += rad;
    }

    void SetST(const float s, const float t);
    void SetPosition(const float &x1, const float &y1);
    void GetPosition(float &x1, float &y1);
    void SetSize(float s1, float s2);
    void GetSize(float &x1, float &y1);
    void SetRotation(const float &rot);
    void GetRotation(float &rot);
    void ReadTexture(FILE *f);

    //float &Rotation(){return rotation;};
    Texture *getTexture() {
        return surface;
    }
};

#endif //VEGA_STRIKE_ENGINE_OBJCONV_SPRITE_H

/*
 * sprite.h
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


#ifndef VEGA_STRIKE_ENGINE_GFX_SPRITE_H
#define VEGA_STRIKE_ENGINE_GFX_SPRITE_H

#include "gfxlib.h"

#include "quaternion.h"

#include "audio/Types.h"
#include "audio/Source.h"

namespace VSFileSystem {
class VSFile;
};

class Texture;

class VSSprite {
    float xcenter;
    float ycenter;
    float widtho2;
    float heighto2;
    float maxs, maxt;
    float rotation;
    Texture *surface;
    bool isAnimation;

//For private use only
    VSSprite() : surface(nullptr), isAnimation(false), rotation(0.0F), maxs(0.0F), maxt(0.0F), heighto2(0.0F), widtho2(0.0F), xcenter(0.0F), ycenter(0.0F) {
    }

public:
//Construct a sprite out of a spritefile
    VSSprite(const char *file, enum FILTER texturefilter = BILINEAR, GFXBOOL force = GFXFALSE);
//Construct a sprite out of a preloaded texture
//@Note will take ownership of 'surface'
    VSSprite(Texture *surface,
            float xcenter,
            float ycenter,
            float width,
            float height,
            float s = 0.f,
            float t = 0.f,
            bool isAnimation = false);
    VSSprite(const VSSprite &source);
    ~VSSprite();
//Return true if sprite was loaded successfully
    bool LoadSuccess() const;
    void Draw();
/**
 * Draw at specified coordinates given by 4 endpoints.
 *
 * @param ll lower-left corner
 * @param lr lower-right corner
 * @param ur upper-right corner
 * @param ul upper-left corner
 * @note Disregards sprite position but not maxs/maxt coordinates.
 */
    void DrawHere(Vector &ll, Vector &lr, Vector &ur, Vector &ul);

//Add specified rotation to an already-rotated sprite
    void Rotate(const float &rad) {
        rotation += rad;
    }

    void SetRotation(const float &rot);
    void GetRotation(float &rot);
//Loads the sprite's texture from the given file
//@deprecated Unused?
    void ReadTexture(VSFileSystem::VSFile *f);
    void GetST(float &s, float &t);
    void SetST(const float s, const float t);
    void SetTime(double newtime);
    void SetPosition(const float &x1, const float &y1);
    void GetPosition(float &x1, float &y1);
    void SetSize(float s1, float s2);
    void GetSize(float &x1, float &y1);
    void SetTimeSource(SharedPtr<Audio::Source> source);
    void ClearTimeSource();
    bool Done() const;
    void Reset();
    SharedPtr<Audio::Source> GetTimeSource() const;

//float &Rotation(){return rotation;};
    Texture *getTexture() {
        return surface;
    }

    const Texture *getTexture() const {
        return surface;
    }
};

#endif //VEGA_STRIKE_ENGINE_GFX_SPRITE_H

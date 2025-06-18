/*
 * animation.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * ace123, surfdargent, klaussfreire, jacks, dan_w, pyramid3d
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
#ifndef VEGA_STRIKE_ENGINE_GFX_ANIMATION_H
#define VEGA_STRIKE_ENGINE_GFX_ANIMATION_H

#include "root_generic/lin_time.h"

#include "gfx_generic/vec.h"

#include <stack>

#include "gfx_generic/quaternion.h"

#include "gfx/ani_texture.h"

namespace VSFileSystem {
class VSFile;
};

class Animation : public AnimatedTexture {
    GFXColor mycolor;

    Matrix local_transformation;

    float height; //half the height so you can do fancy vector translatons to campspace

    float width;

    unsigned char options;

    void InitAnimation();

public:
    Animation();

    Animation(VSFileSystem::VSFile *f, bool Rep = 0, float priority = .1, enum FILTER ismipmapped = MIPMAP,
            bool camorient =
            false, bool appear_near_by_radius = false, const GFXColor &col = GFXColor(1, 1, 1,
            1));

    Animation(const char *, bool Rep = 0, float priority = .1, enum FILTER ismipmapped = MIPMAP, bool camorient = false,
            bool appear_near_by_radius = false, const GFXColor &col = GFXColor(1, 1, 1,
            1));

    ~Animation() override;

    void Draw();

    void SetFaceCam(bool face);

    void SetInterpolate(bool interp);

    bool CalculateOrientation(Matrix &result);

    void DrawNow(const Matrix &final_orientation);

    void DrawNoTransform(bool cross = true, bool blendoption = false);

    void DrawAsVSSprite(class VSSprite *spr);

    static void ProcessDrawQueue(std::vector<Animation *> &, float);

    static void ProcessDrawQueue();

    static bool NeedsProcessDrawQueue();

    static void ProcessFarDrawQueue(float);

    static bool NeedsProcessFarDrawQueue();

    void SetDimensions(float wid, float hei);

    void GetDimensions(float &wid, float &hei);

    QVector Position();

    void SetPosition(const QVector &);

    void SetOrientation(const Vector &p, const Vector &q, const Vector &r);
};

#endif //VEGA_STRIKE_ENGINE_GFX_ANIMATION_H

/*
 * bubble_display.h
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
// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef VEGA_STRIKE_ENGINE_GFX_RADAR_BUBBLE_DISPLAY_H
#define VEGA_STRIKE_ENGINE_GFX_RADAR_BUBBLE_DISPLAY_H

#include <queue>
#include "sensor.h"
#include "radar.h"
#include "dual_display.h"

#include <memory>

struct GFXColor;

namespace Radar {

struct ViewArea;

class BubbleDisplay : public DualDisplayBase {
public:
    BubbleDisplay();
    virtual ~BubbleDisplay();

    void Draw(const Sensor &, VSSprite *, VSSprite *);

    void OnDockEnd();
    void OnJumpBegin();
    void OnJumpEnd();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

protected:
    typedef std::vector<float> ZoomSequence;

    void DrawBackground(const ViewArea &, float);
    void DrawTrack(const Sensor &, const ViewArea &, const Track &);
    void DrawTargetMarker(const Vector &, const GFXColor &, float);

    void Animate();
    void PrepareAnimation(const ZoomSequence &);

protected:
    const float innerSphere;
    const float outerSphere;
    float sphereZoom;
    float radarTime;
    float currentTargetMarkerSize;

    struct AnimationItem {
        float sphereZoom;
        float duration;
    };
    typedef std::queue<AnimationItem> AnimationCollection;
    AnimationCollection animation;
    float lastAnimationTime;
    ZoomSequence explodeSequence;
    ZoomSequence implodeSequence;
};

} // namespace Radar

#endif //VEGA_STRIKE_ENGINE_GFX_RADAR_BUBBLE_DISPLAY_H

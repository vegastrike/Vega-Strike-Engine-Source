/*
 * plane_display.h
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
// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef VEGA_STRIKE_ENGINE_GFX_RADAR_PLANE_DISPLAY_H
#define VEGA_STRIKE_ENGINE_GFX_RADAR_PLANE_DISPLAY_H

#include <vector>
#include <queue>
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "sensor.h"
#include "dual_display.h"

class VSSprite;
struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

namespace Radar {

struct ViewArea;

class PlaneDisplay : public DualDisplayBase {
public:
    PlaneDisplay();

    void Draw(const Sensor &sensor, VSSprite *, VSSprite *);

    void OnDockEnd();
    void OnJumpEnd();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

protected:
    typedef std::vector<float> AngleSequence;

    void DrawGround(const Sensor &, const ViewArea &);
    void DrawNear(const Sensor &, const Sensor::TrackCollection &);
    void DrawDistant(const Sensor &, const Sensor::TrackCollection &);
    void DrawTrack(const Sensor &, const ViewArea &, const Track &, float);
    void DrawTarget(Track::Type::Value, const Vector &, const Vector &, float, const GFXColor &);
    void DrawTargetMarker(const Vector &, const Vector &, const Vector &, float, const GFXColor &, bool);

    void Animate();
    void PrepareAnimation(const Vector &,
            const Vector &,
            const AngleSequence &,
            const AngleSequence &,
            const AngleSequence &);

    void CalculateRotation();
    Vector Projection(const ViewArea &, const Vector &);

protected:
    Vector finalCameraAngle;
    Vector currentCameraAngle;
    Vector xrotation;
    Vector yrotation;
    Vector zrotation;
    std::vector<Vector> groundPlane;
    float radarTime;

    // Primitive animation system
    struct AnimationItem {
        float duration;
        Vector position;
    };
    typedef std::queue<AnimationItem> AnimationCollection;
    AnimationCollection animation;
    float lastAnimationTime;
    AngleSequence nothingSequence;
    AngleSequence bounceSequence;
    AngleSequence cosineSequence;
};

} // namespace Radar

#endif //VEGA_STRIKE_ENGINE_GFX_RADAR_PLANE_DISPLAY_H

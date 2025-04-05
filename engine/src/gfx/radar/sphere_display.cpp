// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * sphere_display.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#include "root_generic/lin_time.h" // GetElapsedTime
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "src/gfxlib.h"
#include "viewarea.h"
#include "sphere_display.h"
#include "src/physics.h"
#include "root_generic/configxml.h"

#define TRACK_SIZE 2.0

namespace {

float GetDangerRate(Radar::Sensor::ThreatLevel::Value threat) {
    using namespace Radar;

    switch (threat) {
        case Sensor::ThreatLevel::High:
            return 20.0; // Fast pulsation

        case Sensor::ThreatLevel::Medium:
            return 7.5; // Slow pulsation

        default:
            return 0.0; // No pulsation
    }
}

} // anonymous namespace

namespace Radar {

struct SphereDisplay::Impl {
    VertexBuilder<float, 3, 0, 3> points;
    VertexBuilder<float, 3, 0, 3> lines;
    VertexBuilder<> thinlines;

    void clear() {
        points.clear();
        lines.clear();
        thinlines.clear();
    }
};

SphereDisplay::SphereDisplay()
        : impl(new SphereDisplay::Impl), innerSphere(0.98), radarTime(0.0) {
}

SphereDisplay::~SphereDisplay() {
}

void SphereDisplay::Draw(const Sensor &sensor,
        VSSprite *frontSprite,
        VSSprite *rearSprite) {
    assert(frontSprite || rearSprite); // There should be at least one radar display

    radarTime += GetElapsedTime();

    impl->clear();

    leftRadar.SetSprite(frontSprite);
    rightRadar.SetSprite(rearSprite);

    if (frontSprite) {
        frontSprite->Draw();
    }
    if (rearSprite) {
        rearSprite->Draw();
    }

    Sensor::TrackCollection tracks = sensor.FindTracksInRange();

    // FIXME: Consider using std::sort instead of the z-buffer
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);

    DrawBackground(sensor, leftRadar);
    DrawBackground(sensor, rightRadar);

    for (Sensor::TrackCollection::const_iterator it = tracks.begin(); it != tracks.end(); ++it) {
        const bool draw_both = vega_config::config->graphics.hud.draw_blips_on_both_radar;
        if (it->GetPosition().z < 0 || draw_both) {
            // Draw tracks behind the ship
            DrawTrack(sensor, rightRadar, *it, true);
        }
        if (it->GetPosition().z >= 0 || draw_both) {
            // Draw tracks in front of the ship
            DrawTrack(sensor, leftRadar, *it);
        }
    }

    GFXPointSize(TRACK_SIZE);
    GFXDraw(GFXPOINT, impl->points);

    GFXLineWidth(TRACK_SIZE);
    GFXDraw(GFXLINE, impl->lines);

    GFXLineWidth(1);
    GFXDraw(GFXLINE, impl->thinlines);

    GFXPointSize(1);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
}

void SphereDisplay::DrawTrack(const Sensor &sensor,
        const ViewArea &radarView,
        const Track &track,
        bool negate_z) {
    if (!radarView.IsActive()) {
        return;
    }

    GFXColor color = sensor.GetColor(track);

    Vector position = track.GetPosition();
    if (negate_z) {
        position.z = -position.z;
    }
    if (position.z < 0) {
        if (vega_config::config->graphics.hud.show_negative_blips_as_positive) {
            position.z = -position.z;
        } else {
            position.z = 0.125;
        }
    }

    // FIXME: Jitter only on boundary, not in center
    if (sensor.InsideNebula()) {
        Jitter(0.02, 0.04, position);
    } else {
        const bool isNebula = (track.GetType() == Track::Type::Nebula);
        const bool isEcmActive = track.HasActiveECM();
        if (isNebula || isEcmActive) {
            float error = 0.02 * TRACK_SIZE;
            Jitter(error, error, position);
        }
    }

    // The magnitude is used to calculate the unit vector. With subtle scaling
    // of the magnitude we generate a unit vector whose length will vary from
    // innerSphere to 1.0, depending on the distance to the object. Combined
    // with the OpenGL z-buffering, this will ensure that close tracks are drawn
    // on top of distant tracks.
    float magnitude = position.Magnitude();
    float scaleFactor = 0.0; // [0; 1] where 0 = border, 1 = center
    const float maxRange = sensor.GetMaxRange();
    if (magnitude <= maxRange) {
        // [innerSphere; 1]
        scaleFactor = (1.0 - innerSphere) * (maxRange - magnitude) / maxRange;
        magnitude /= (1.0 - scaleFactor);
    }
    Vector scaledPosition = Vector(-position.x, position.y, position.z) / magnitude;

    Vector head = radarView.Scale(scaledPosition);

    GFXColor headColor = color;
    if (sensor.UseThreatAssessment()) {
        float dangerRate = GetDangerRate(sensor.IdentifyThreat(track));
        if (dangerRate > 0.0) {
            // Blinking track
            headColor.a *= cosf(dangerRate * radarTime);
        }
    }
    // Fade out dying ships
    if (track.IsExploding()) {
        headColor.a *= (1.0 - track.ExplodingProgress());
    }

    if (sensor.IsTracking(track)) {
        DrawTargetMarker(head, headColor, TRACK_SIZE);
    }

    impl->points.insert(GFXColorVertex(head, headColor));
}

void SphereDisplay::DrawTargetMarker(const Vector &position, const GFXColor &color, float trackSize) {
    // Crosshair
    const float crossSize = 8.0;
    const float xcross = crossSize / g_game.x_resolution;
    const float ycross = crossSize / g_game.y_resolution;

    // The crosshair wiggles as it moves around. The wiggling is less noticable
    // when the crosshair is drawn with the smooth option.
    impl->lines.insert(position.x + xcross, position.y, 0.0f, color);
    impl->lines.insert(position.x - xcross, position.y, 0.0f, color);
    impl->lines.insert(position.x, position.y - ycross, 0.0f, color);
    impl->lines.insert(position.x, position.y + ycross, 0.0f, color);
}

void SphereDisplay::DrawBackground(const Sensor &sensor, const ViewArea &radarView) {
    // Split crosshair

    if (!radarView.IsActive()) {
        return;
    }

    GFXColor groundColor = radarView.GetColor();

    float velocity = sensor.GetPlayer()->GetWarpVelocity().Magnitude();
    float logvelocity = 3.0; // std::log10(1000.0);
    if (velocity > 1000.0) {
        // Max logvelocity is log10(speed_of_light) = 10.46
        logvelocity = std::log10(velocity);
    }
    const float size = 3.0 * logvelocity; // [9; 31]
    const float xground = size / g_game.x_resolution;
    const float yground = size / g_game.y_resolution;
    Vector center = radarView.Scale(Vector(0.0, 0.0, 0.0));

    impl->thinlines.insert(center.x - 2.0 * xground, center.y, center.z, groundColor);
    impl->thinlines.insert(center.x - xground, center.y, center.z, groundColor);
    impl->thinlines.insert(center.x + 2.0 * xground, center.y, center.z, groundColor);
    impl->thinlines.insert(center.x + xground, center.y, center.z, groundColor);
    impl->thinlines.insert(center.x, center.y - 2.0 * yground, center.z, groundColor);
    impl->thinlines.insert(center.x, center.y - yground, center.z, groundColor);
    impl->thinlines.insert(center.x, center.y + 2.0 * yground, center.z, groundColor);
    impl->thinlines.insert(center.x, center.y + yground, center.z, groundColor);
}

} // namespace Radar

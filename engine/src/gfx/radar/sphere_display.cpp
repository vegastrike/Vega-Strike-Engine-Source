// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * sphere_display.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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


#include "root_generic/lin_time.h" // GetElapsedTime
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "src/gfxlib.h"
#include "viewarea.h"
#include "sphere_display.h"
#include "src/physics.h"
#include "root_generic/configxml.h"

#include <map>

#define TRACK_SIZE 2.0
#define POINT_SIZE_GRANULARITY 0.5

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
    typedef VertexBuilder<float, 3, 0, 3> PointBuffer;
    typedef std::map<unsigned int, PointBuffer> PointBufferMap;

    PointBufferMap pointmap;
    VertexBuilder<float, 3, 0, 3> lines;
    VertexBuilder<> thinlines;

    PointBuffer &getPointBuffer(float size) {
        int isize = int(size / POINT_SIZE_GRANULARITY);
        if (isize < 1) {
            isize = 1;
        }

        PointBufferMap::iterator it = pointmap.find(isize);
        if (it == pointmap.end()) {
            it = pointmap.insert(std::pair<unsigned int, PointBuffer>(isize, PointBuffer())).first;
        }
        return it->second;
    }

    void clear() {
        for (PointBufferMap::iterator it = pointmap.begin(); it != pointmap.end(); ++it) {
            it->second.clear();
        }

        lines.clear();
        thinlines.clear();
    }

    void flushPoints() {
        for (PointBufferMap::reverse_iterator it = pointmap.rbegin(); it != pointmap.rend(); ++it) {
            PointBuffer &points = it->second;
            if (points.size() > 0) {
                GFXPointSize(it->first * POINT_SIZE_GRANULARITY);
                GFXDraw(GFXPOINT, points);
            }
        }
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
        const bool draw_both = configuration().graphics.hud.draw_blips_on_both_radar;
        if (it->GetPosition().z < 0 || draw_both) {
            // Draw tracks behind the ship
            DrawTrack(sensor, rightRadar, *it, true);
        }
        if (it->GetPosition().z >= 0 || draw_both) {
            // Draw tracks in front of the ship
            DrawTrack(sensor, leftRadar, *it);
        }
    }

    impl->flushPoints();

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
        if (configuration().graphics.hud.show_negative_blips_as_positive) {
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
    if (sensor.IsRepulsor(track)) {
        // Blinking repulsor blip
        headColor.a *= cosf(kRepulsorBlinkRate * radarTime);
    }
    // Fade out dying ships
    if (track.IsExploding()) {
        headColor.a *= (1.0 - track.ExplodingProgress());
    }

    if (sensor.IsTracking(track)) {
        GFXColor markerColor = headColor;
        if (sensor.IsSpecActive()) {
            markerColor = sensor.GetSpecTargetColor();
        }
        DrawTargetMarker(head, markerColor, TRACK_SIZE);
    }

    float blipSize = TRACK_SIZE;
    const double repulsor_effect = sensor.GetRepulsorEffect(track);
    if (repulsor_effect > 0.0) {
        // Size the blip by how strongly the object compresses SPEC
        blipSize = RepulsorBlipSize(repulsor_effect);
    }
    impl->getPointBuffer(blipSize).insert(GFXColorVertex(head, headColor));
}

void SphereDisplay::DrawTargetMarker(const Vector &position, const GFXColor &color, float trackSize) {
    // Crosshair
    const float crossSize = 8.0;
    const float xcross = crossSize / configuration().graphics.resolution_x;
    const float ycross = crossSize / configuration().graphics.resolution_y;

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
    const float xground = size / configuration().graphics.resolution_x;
    const float yground = size / configuration().graphics.resolution_y;
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

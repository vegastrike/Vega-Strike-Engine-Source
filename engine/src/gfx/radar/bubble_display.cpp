// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * bubble_display.cpp
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


#include <cmath>
#include <vector>
#include <map>
#include <boost/assign/std/vector.hpp>
#include "root_generic/lin_time.h" // GetElapsedTime
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "src/gfxlib.h"
#include "viewarea.h"
#include "bubble_display.h"

#include <limits>

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

struct BubbleDisplay::Impl {
    typedef VertexBuilder<float, 3, 0, 4> LineBuffer;
    typedef VertexBuilder<float, 3, 0, 4> PointBuffer;
    typedef std::vector<unsigned short> LineElements;

    typedef std::map<unsigned int, PointBuffer> PointBufferMap;

    LineBuffer lines;
    LineElements lineIndices;
    PointBufferMap pointmap;

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
        lineIndices.clear();
    }

    void flush() {
        for (Impl::PointBufferMap::reverse_iterator it = pointmap.rbegin(); it != pointmap.rend(); ++it) {
            Impl::PointBuffer &points = it->second;
            if (points.size() > 0) {
                GFXPointSize(it->first * POINT_SIZE_GRANULARITY);
                GFXDraw(GFXPOINT, points);
            }
        }

        GFXLineWidth(1);
        GFXDrawElements(GFXLINE, lines, lineIndices);
    }
};

BubbleDisplay::BubbleDisplay()
        : impl(new BubbleDisplay::Impl),
        innerSphere(0.45),
        outerSphere(1.0),
        sphereZoom(1.0),
        radarTime(0.0),
        currentTargetMarkerSize(0.0),
        lastAnimationTime(0.0) {
    using namespace boost::assign; // vector::operator+=
    explodeSequence +=
            0.0, 0.0001, 0.0009, 0.0036, 0.0100, 0.0225, 0.0441, 0.0784, 0.1296, 0.2025, 0.3025, 0.4356, 0.6084, 0.8281, 1.0, 0.8713, 0.7836, 0.7465, 0.7703, 0.8657, 1.0, 0.9340, 0.9595, 1.0, 0.9659, 1.0;
    implodeSequence +=
            1.0, 0.9999, 0.9991, 0.9964, 0.9900, 0.9775, 0.9559, 0.9216, 0.8704, 0.7975, 0.6975, 0.5644, 0.3916, 0.1719, 0.0, 0.1287, 0.2164, 0.2535, 0.2297, 0.1343, 0.0, 0.0660, 0.0405, 0.0, 0.0341, 0.0;
}

BubbleDisplay::~BubbleDisplay() {
}

void BubbleDisplay::PrepareAnimation(const ZoomSequence &sequence) {
    AnimationItem firstItem;
    firstItem.duration = 0.0;
    firstItem.sphereZoom = sequence[0];
    animation.push(firstItem);

    float duration = 2.0;
    for (ZoomSequence::size_type i = 1; i < sequence.size(); ++i) {
        AnimationItem item;
        item.duration = duration;
        item.sphereZoom = sequence[i];
        animation.push(item);
        duration = 0.05;
    }
}

void BubbleDisplay::OnDockEnd() {
    PrepareAnimation(explodeSequence);
}

void BubbleDisplay::OnJumpBegin() {
    PrepareAnimation(implodeSequence);
}

void BubbleDisplay::OnJumpEnd() {
    PrepareAnimation(explodeSequence);
}

void BubbleDisplay::Animate() {
    if (!animation.empty()) {
        if (radarTime > lastAnimationTime + animation.front().duration) {
            sphereZoom = animation.front().sphereZoom;
            animation.pop();
            lastAnimationTime = radarTime;
        }
    }
}

void BubbleDisplay::Draw(const Sensor &sensor,
        VSSprite *frontSprite,
        VSSprite *rearSprite) {
    assert(frontSprite || rearSprite); // There should be at least one radar display

    radarTime += GetElapsedTime();

    leftRadar.SetSprite(frontSprite);
    rightRadar.SetSprite(rearSprite);

    impl->clear();

    if (frontSprite) {
        frontSprite->Draw();
    }
    if (rearSprite) {
        rearSprite->Draw();
    }

    Sensor::TrackCollection tracks = sensor.FindTracksInRange();

    Animate();

    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXEnable(SMOOTH);

    for (Sensor::TrackCollection::const_iterator it = tracks.begin(); it != tracks.end(); ++it) {
        if (it->GetPosition().z < 0) {
            // Draw tracks behind the ship
            DrawTrack(sensor, rightRadar, *it);
        } else {
            // Draw tracks in front of the ship
            DrawTrack(sensor, leftRadar, *it);
        }
    }
    DrawBackground(leftRadar, currentTargetMarkerSize);
    DrawBackground(rightRadar, currentTargetMarkerSize);

    impl->flush();

    GFXPointSize(1);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    GFXDisable(SMOOTH);
}

void BubbleDisplay::DrawTrack(const Sensor &sensor,
        const ViewArea &radarView,
        const Track &track) {
    if (!radarView.IsActive()) {
        return;
    }

    GFXColor color = sensor.GetColor(track);

    Vector position = track.GetPosition();
    if (position.z < 0) {
        position.z = -position.z;
    }

    float magnitude = position.Magnitude();
    float scaleFactor = 0.0; // [0; 1] where 0 = border, 1 = center
    float maxRange = sensor.GetMaxRange();
    if (magnitude <= maxRange) {
        // [innerSphere; outerSphere]
        scaleFactor = (outerSphere - innerSphere) * ((maxRange - magnitude) / maxRange);
        magnitude /= (1.0 - scaleFactor);
    }

    if (sensor.InsideNebula()) {
        magnitude /= (1.0 - 0.04 * Jitter(0.0, 1.0));
    }
    Vector scaledPosition = sphereZoom * Vector(-position.x, position.y, position.z) / magnitude;

    Vector head = radarView.Scale(scaledPosition);

    GFXColor headColor = color;

    headColor.a *= 0.2 + scaleFactor * (1.0 - 0.2); // [0;1] => [0.1;1]
    if (sensor.UseThreatAssessment()) {
        float dangerRate = GetDangerRate(sensor.IdentifyThreat(track));
        if (dangerRate > 0.0) {
            // Blinking blip
            headColor.a *= cosf(dangerRate * radarTime);
        }
    }

    // Fade out dying ships
    if (track.IsExploding()) {
        headColor.a *= (1.0 - track.ExplodingProgress());
    }

    float trackSize = std::max(1.0f, std::log10(track.GetSize()));
    if (track.GetType() != Track::Type::Cargo) {
        trackSize += 1.0;
    }

    if (sensor.IsTracking(track)) {
        currentTargetMarkerSize = trackSize;
        DrawTargetMarker(head, headColor, trackSize);
    }

    const bool isNebula = (track.GetType() == Track::Type::Nebula);
    const bool isEcmActive = track.HasActiveECM();
    if (isNebula || isEcmActive) {
        // Vary size between 50% and 150%
        trackSize *= Jitter(0.5, 1.0);
    }

    impl->getPointBuffer(trackSize).insert(GFXColorVertex(head, headColor));
}

void BubbleDisplay::DrawTargetMarker(const Vector &position, const GFXColor &color, float trackSize) {
    // Split octagon
    float size = 3.0 * std::max(trackSize, 3.0f);
    float xsize = size / configuration().graphics.resolution_x;
    float ysize = size / configuration().graphics.resolution_y;

    Impl::LineElements::value_type base_index = Impl::LineElements::value_type(impl->lines.size());

    // Don't overflow the index type
    if (base_index < (std::numeric_limits<Impl::LineElements::value_type>::max() - 8)) {
        impl->lines.insert(position.x - xsize / 2, position.y - ysize, position.z, color);
        impl->lines.insert(position.x - xsize, position.y - ysize / 2, position.z, color);
        impl->lines.insert(position.x - xsize, position.y + ysize / 2, position.z, color);
        impl->lines.insert(position.x - xsize / 2, position.y + ysize, position.z, color);
        impl->lines.insert(position.x + xsize / 2, position.y - ysize, position.z, color);
        impl->lines.insert(position.x + xsize, position.y - ysize / 2, position.z, color);
        impl->lines.insert(position.x + xsize, position.y + ysize / 2, position.z, color);
        impl->lines.insert(position.x + xsize / 2, position.y + ysize, position.z, color);
        impl->lineIndices.push_back(base_index + 0);
        impl->lineIndices.push_back(base_index + 1);
        impl->lineIndices.push_back(base_index + 1);
        impl->lineIndices.push_back(base_index + 2);
        impl->lineIndices.push_back(base_index + 2);
        impl->lineIndices.push_back(base_index + 3);
        impl->lineIndices.push_back(base_index + 4);
        impl->lineIndices.push_back(base_index + 5);
        impl->lineIndices.push_back(base_index + 5);
        impl->lineIndices.push_back(base_index + 6);
        impl->lineIndices.push_back(base_index + 6);
        impl->lineIndices.push_back(base_index + 7);
    }
}

void BubbleDisplay::DrawBackground(const ViewArea &radarView, float trackSize) {
    if (!radarView.IsActive()) {
        return;
    }

    GFXColor groundColor = radarView.GetColor();

    // Split octagon
    float size = 3.0 * std::max(trackSize, 3.0f);
    float xground = size / configuration().graphics.resolution_x;
    float yground = size / configuration().graphics.resolution_y;
    Vector center = radarView.Scale(Vector(0.0, 0.0, 0.0));

    Impl::LineElements::value_type base_index = Impl::LineElements::value_type(impl->lines.size());

    // Don't overflow the index type
    if (base_index < (std::numeric_limits<Impl::LineElements::value_type>::max() - 8)) {
        impl->lines.insert(center.x - xground, center.y - yground / 2, center.z, groundColor);
        impl->lines.insert(center.x - xground / 2, center.y - yground, center.z, groundColor);
        impl->lines.insert(center.x + xground / 2, center.y - yground, center.z, groundColor);
        impl->lines.insert(center.x + xground, center.y - yground / 2, center.z, groundColor);
        impl->lines.insert(center.x - xground, center.y + yground / 2, center.z, groundColor);
        impl->lines.insert(center.x - xground / 2, center.y + yground, center.z, groundColor);
        impl->lines.insert(center.x + xground / 2, center.y + yground, center.z, groundColor);
        impl->lines.insert(center.x + xground, center.y + yground / 2, center.z, groundColor);
        impl->lineIndices.push_back(base_index + 0);
        impl->lineIndices.push_back(base_index + 1);
        impl->lineIndices.push_back(base_index + 1);
        impl->lineIndices.push_back(base_index + 2);
        impl->lineIndices.push_back(base_index + 2);
        impl->lineIndices.push_back(base_index + 3);
        impl->lineIndices.push_back(base_index + 4);
        impl->lineIndices.push_back(base_index + 5);
        impl->lineIndices.push_back(base_index + 5);
        impl->lineIndices.push_back(base_index + 6);
        impl->lineIndices.push_back(base_index + 6);
        impl->lineIndices.push_back(base_index + 7);
    }
}

} // namespace Radar

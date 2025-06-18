// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * plane_display.cpp
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
#include <algorithm>
#include <vector>
#include <boost/assign/std/vector.hpp>
#include "root_generic/lin_time.h" // GetElapsedTime
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "src/gfxlib.h"
#include "gfx_generic/quaternion.h"
#include "viewarea.h"
#include "plane_display.h"

#include <limits>

#define POINT_SIZE_GRANULARITY 0.5

namespace {

float Degree2Radian(float angle) {
    const float ratio = M_PI / 180.0;
    return angle * ratio;
}

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

struct PlaneDisplay::Impl {
    typedef VertexBuilder<float, 3, 0, 4> PointBuffer;
    typedef VertexBuilder<float, 3, 0, 4> LineBuffer;
    typedef VertexBuilder<float, 3, 0, 4> PolyBuffer;
    typedef std::vector<unsigned short> ElementBuffer;
    typedef std::map<unsigned int, PointBuffer> PointBufferMap;
    PointBufferMap headsmap;

    VertexBuilder<> ground;
    LineBuffer legs;
    LineBuffer diamonds;
    ElementBuffer diamondIndices;
    PointBuffer heads;
    PolyBuffer areas;

    PointBuffer &getHeadBuffer(float size) {
        int isize = int(size / POINT_SIZE_GRANULARITY);
        if (isize < 1) {
            isize = 1;
        }

        PointBufferMap::iterator it = headsmap.find(isize);
        if (it == headsmap.end()) {
            it = headsmap.insert(std::pair<unsigned int, PointBuffer>(isize, PointBuffer())).first;
        }
        return it->second;
    }

    void clear() {
        ground.clear();
        legs.clear();
        areas.clear();
        diamonds.clear();
        diamondIndices.clear();

        for (PointBufferMap::iterator it = headsmap.begin(); it != headsmap.end(); ++it) {
            it->second.clear();
        }
    }

    void flush() {
        GFXDraw(GFXTRI, areas);

        GFXLineWidth(0.2);
        GFXDraw(GFXLINE, legs);

        for (Impl::PointBufferMap::reverse_iterator it = headsmap.rbegin(); it != headsmap.rend(); ++it) {
            Impl::PointBuffer &points = it->second;
            if (points.size() > 0) {
                GFXPointSize(it->first * POINT_SIZE_GRANULARITY);
                GFXDraw(GFXPOINT, points);
            }
        }

        GFXLineWidth(1);
        GFXDrawElements(GFXLINE, diamonds, diamondIndices);
    }
};

PlaneDisplay::PlaneDisplay()
        : impl(new PlaneDisplay::Impl),
        finalCameraAngle(Degree2Radian(30), Degree2Radian(0), Degree2Radian(0)),
        currentCameraAngle(finalCameraAngle),
        radarTime(0.0),
        lastAnimationTime(0.0) {
    using namespace boost::assign; // vector::operator+=

    CalculateRotation();

    // Calculate ground plane
    const float edges = 32;
    const float full = 2 * M_PI;
    const float step = full / edges;
    for (float angle = 0.0; angle < full; angle += step) {
        groundPlane.push_back(Vector(cosf(angle), 0.0f, sinf(angle)));
    }

    // Sequences start in 1 and ends in 0
    nothingSequence += 0.0;
    bounceSequence +=
            1.0, 0.9999, 0.9991, 0.9964, 0.9900, 0.9775, 0.9559, 0.9216, 0.8704, 0.7975, 0.6975, 0.5644, 0.3916, 0.1719, 0.0, 0.1287, 0.2164, 0.2535, 0.2297, 0.1343, 0.0, 0.0660, 0.0405, 0.0, 0.0341, 0.0;
    cosineSequence +=
            1.0, 0.999391, 0.997564, 0.994522, 0.990268, 0.984808, 0.978148, 0.970296, 0.961262, 0.951057, 0.939693, 0.927184, 0.913545, 0.898794, 0.882948, 0.866025, 0.848048, 0.829038, 0.809017, 0.788011, 0.766044, 0.743145, 0.71934, 0.694658, 0.669131, 0.642788, 0.615662, 0.587785, 0.559193, 0.529919, 0.5, 0.469472, 0.438371, 0.406737, 0.374607, 0.34202, 0.309017, 0.275637, 0.241922, 0.207911, 0.173648, 0.139173, 0.104528, 0.069756, 0.034899, 0.0;
}

void PlaneDisplay::CalculateRotation() {
    const float cosx = cosf(currentCameraAngle.x);
    const float cosy = cosf(currentCameraAngle.y);
    const float cosz = cosf(currentCameraAngle.z);
    const float sinx = sinf(currentCameraAngle.x);
    const float siny = sinf(currentCameraAngle.y);
    const float sinz = sinf(currentCameraAngle.z);

    xrotation = Vector(cosy * cosz,
            sinx * siny * cosz - cosx * sinz,
            cosx * siny * cosz + sinx * sinz);
    yrotation = Vector(cosy * sinz,
            cosx * cosz + sinx * siny * sinz,
            cosx * siny * sinz - sinx * cosz);
    zrotation = Vector(-siny,
            sinx * cosy,
            cosx * cosy);
}

void PlaneDisplay::PrepareAnimation(const Vector &fromAngle,
        const Vector &toAngle,
        const AngleSequence &xsequence,
        const AngleSequence &ysequence,
        const AngleSequence &zsequence) {
    AnimationItem firstItem;
    firstItem.duration = 0.0;
    firstItem.position = fromAngle;
    animation.push(firstItem);

    float duration = 2.0;

    // Use the longest-running sequence and zero-pad shorter sequuences
    AngleSequence::size_type longestSequenceSize = xsequence.size();
    longestSequenceSize = std::max(longestSequenceSize, ysequence.size());
    longestSequenceSize = std::max(longestSequenceSize, zsequence.size());
    for (AngleSequence::size_type i = 0; i < longestSequenceSize; ++i) {
        float xentry = (i < xsequence.size()) ? xsequence[i] : 0.0;
        float yentry = (i < ysequence.size()) ? ysequence[i] : 0.0;
        float zentry = (i < zsequence.size()) ? zsequence[i] : 0.0;
        float xangle = toAngle.x + xentry * (fromAngle.x - toAngle.x);
        float yangle = toAngle.y + yentry * (fromAngle.y - toAngle.y);
        float zangle = toAngle.z + zentry * (fromAngle.z - toAngle.z);
        AnimationItem item;
        item.duration = duration;
        item.position = Vector(xangle, yangle, zangle);
        animation.push(item);
        duration = 0.05;
    }

    AnimationItem finalItem;
    finalItem.duration = duration;
    finalItem.position = toAngle;
    animation.push(finalItem);
}

void PlaneDisplay::OnDockEnd() {
    // Bounce from upright position
    Vector undockCameraAngle(Degree2Radian(90), finalCameraAngle.y, finalCameraAngle.z);
    PrepareAnimation(undockCameraAngle, finalCameraAngle, bounceSequence, nothingSequence, nothingSequence);
}

void PlaneDisplay::OnJumpEnd() {
    // Full rotation around y-axis
    Vector jumpCameraAngle(finalCameraAngle.x, Degree2Radian(360), finalCameraAngle.z);
    PrepareAnimation(jumpCameraAngle, finalCameraAngle, nothingSequence, cosineSequence, nothingSequence);
}

void PlaneDisplay::Draw(const Sensor &sensor,
        VSSprite *nearSprite,
        VSSprite *distantSprite) {
    assert(nearSprite || distantSprite); // There should be at least one radar display

    radarTime += GetElapsedTime();

    leftRadar.SetSprite(nearSprite);
    rightRadar.SetSprite(distantSprite);

    if (nearSprite) {
        nearSprite->Draw();
    }
    if (distantSprite) {
        distantSprite->Draw();
    }

    Sensor::TrackCollection tracks = sensor.FindTracksInRange();

    Animate();

    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXEnable(SMOOTH);

    DrawNear(sensor, tracks);
    DrawDistant(sensor, tracks);

    GFXPointSize(1);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    GFXDisable(SMOOTH);
}

void PlaneDisplay::Animate() {
    if (!animation.empty()) {
        if (radarTime > lastAnimationTime + animation.front().duration) {
            currentCameraAngle = animation.front().position;
            CalculateRotation();
            animation.pop();
            lastAnimationTime = radarTime;
        }
    }
}

Vector PlaneDisplay::Projection(const ViewArea &radarView, const Vector &position) {
    // 1. Rotate
    float rx = position.Dot(xrotation);
    float ry = position.Dot(yrotation);
    float rz = position.Dot(zrotation);

    // 2. Project perspective
    // Using the symmetric viewing volume where right = -left and top = -bottom
    // gives us this perspective projection matrix
    //   n/r           0             0             0
    //   0             n/t           0             0
    //   0             0             -(f+n)/(f-n)  -2fn/(f-n)
    //   0             0             -1            0
    // location = M_perspective * rotatedPosition
    const float nearDistance = -0.5; // -0.25 => zoom out, -0.75 => zoom in
    const float farDistance = 0.5;
    const float top = 0.5;
    const float right = 0.5;
    float x = rx * (nearDistance / right);
    float y = ry * (nearDistance / top);
    float z = (rz * (-(farDistance + nearDistance) / (farDistance - nearDistance))
            - 2.0 * farDistance * nearDistance / (farDistance - nearDistance));

    // 3. Scale onto radarView
    return radarView.Scale(Vector(x, y, z));
}

void PlaneDisplay::DrawGround(const Sensor &sensor, const ViewArea &radarView) {
    GFXColor groundColor = radarView.GetColor();
    const float outer = 3.0 / 3.0;
    const float middle = 2.0 / 3.0;
    const float inner = 1.0 / 3.0;

    groundColor.a = 0.1;
    GFXColorf(groundColor);
    GFXLineWidth(0.5);
    impl->ground.clear();
    for (std::vector<Vector>::const_iterator it = groundPlane.begin(); it != groundPlane.end(); ++it) {
        impl->ground.insert(Projection(radarView, outer * (*it)));
    }
    GFXDraw(GFXPOLY, impl->ground);

    groundColor.a = 0.4;
    GFXColorf(groundColor);
    impl->ground.clear();
    for (std::vector<Vector>::const_iterator it = groundPlane.begin(); it != groundPlane.end(); ++it) {
        impl->ground.insert(Projection(radarView, middle * (*it)));
    }
    impl->ground.insert(Projection(radarView, middle * groundPlane.front()));
    GFXDraw(GFXLINESTRIP, impl->ground);

    impl->ground.clear();
    for (std::vector<Vector>::const_iterator it = groundPlane.begin(); it != groundPlane.end(); ++it) {
        impl->ground.insert(Projection(radarView, inner * (*it)));
    }
    impl->ground.insert(Projection(radarView, inner * groundPlane.front()));
    GFXDraw(GFXLINESTRIP, impl->ground);

    groundColor.a = 0.4;
    const float xcone = cosf(sensor.GetLockCone());
    const float zcone = sinf(sensor.GetLockCone());
    const float innerCone = inner;
    const float outerCone = outer;
    Vector leftCone(xcone, 0.0f, zcone);
    Vector rightCone(-xcone, 0.0f, zcone);
    GFXColorf(groundColor);
    impl->ground.clear();
    impl->ground.insert(Projection(radarView, outerCone * leftCone));
    impl->ground.insert(Projection(radarView, innerCone * leftCone));
    impl->ground.insert(Projection(radarView, outerCone * rightCone));
    impl->ground.insert(Projection(radarView, innerCone * rightCone));
    GFXDraw(GFXLINE, impl->ground);
    GFXLineWidth(1);
}

void PlaneDisplay::DrawNear(const Sensor &sensor,
        const Sensor::TrackCollection &tracks) {
    // Draw all near tracks (distance scaled)

    if (!leftRadar.IsActive()) {
        return;
    }

    float maxRange = sensor.GetCloseRange();

    DrawGround(sensor, leftRadar);

    impl->clear();

    for (Sensor::TrackCollection::const_iterator it = tracks.begin(); it != tracks.end(); ++it) {
        if (it->GetDistance() > maxRange) {
            continue;
        }

        DrawTrack(sensor, leftRadar, *it, maxRange);
    }

    impl->flush();
}

void PlaneDisplay::DrawDistant(const Sensor &sensor,
        const Sensor::TrackCollection &tracks) {
    // Draw all near tracks (distance scaled)

    if (!rightRadar.IsActive()) {
        return;
    }

    float minRange = sensor.GetCloseRange();
    float maxRange = sensor.GetMaxRange();

    DrawGround(sensor, rightRadar);

    impl->clear();

    for (Sensor::TrackCollection::const_iterator it = tracks.begin(); it != tracks.end(); ++it) {
        if ((it->GetDistance() < minRange) || (it->GetDistance() > maxRange)) {
            continue;
        }

        DrawTrack(sensor, rightRadar, *it, maxRange);
    }

    impl->flush();
}

void PlaneDisplay::DrawTrack(const Sensor &sensor,
        const ViewArea &radarView,
        const Track &track,
        float maxRange) {
    const Track::Type::Value unitType = track.GetType();
    GFXColor color = sensor.GetColor(track);

    Vector position = track.GetPosition();
    Vector scaledPosition = Vector(position.x, -position.y, position.z) / maxRange;
    if (scaledPosition.Magnitude() > 1.0) {
        return;
    }

    // FIXME: Integrate radar into damage/repair system
    // FIXME: Jitter does not work when entering a nebula
    // FIXME: Jitter does not work close by
    if (sensor.InsideNebula()) {
        Jitter(0.0, 0.01, scaledPosition);
    } else {
        const bool isNebula = (track.GetType() == Track::Type::Nebula);
        const bool isEcmActive = track.HasActiveECM();
        if (isNebula || isEcmActive) {
            const float errorOffset = (scaledPosition.x > 0.0 ? 0.01 : -0.01);
            const float errorRange = 0.03;
            Jitter(errorOffset, errorRange, scaledPosition);
        }
    }

    Vector head = Projection(radarView, scaledPosition);

    Vector scaledGround(scaledPosition.x, 0, scaledPosition.z);
    Vector ground = Projection(radarView, scaledGround);

    const bool isBelowGround = (scaledPosition.y > 0); // Y has been inverted

    // Tracks below ground are muted
    if (isBelowGround) {
        color.a /= 3;
    }
    // and so is cargo
    if (track.GetType() == Track::Type::Cargo) {
        color.a /= 4;
    }

    if (sensor.UseThreatAssessment()) {
        float dangerRate = GetDangerRate(sensor.IdentifyThreat(track));
        if (dangerRate > 0.0) {
            // Blinking track
            color.a *= cosf(dangerRate * radarTime);
        }
    }

    // Fade out dying ships
    if (track.IsExploding()) {
        color.a *= (1.0 - track.ExplodingProgress());
    }

    float trackSize = std::max(1.0f, std::log10(track.GetSize()));
    if (track.GetType() != Track::Type::Cargo) {
        trackSize += 1.0;
    }

    DrawTarget(unitType, head, ground, trackSize, color);

    if (sensor.IsTracking(track)) {
        Vector center = Projection(radarView, Vector(0, 0, 0));
        DrawTargetMarker(head, ground, center, trackSize, color, sensor.UseObjectRecognition());
    }
}

void PlaneDisplay::DrawTarget(Track::Type::Value unitType,
        const Vector &head,
        const Vector &ground,
        float trackSize,
        const GFXColor &color) {
    // Draw leg
    GFXColor legColor = color;
    legColor.a /= 2;
    impl->legs.insert(GFXColorVertex(head, legColor));
    impl->legs.insert(GFXColorVertex(ground, legColor));

    // Draw head
    impl->getHeadBuffer(trackSize).insert(GFXColorVertex(head, color));
}

void PlaneDisplay::DrawTargetMarker(const Vector &head,
        const Vector &ground,
        const Vector &center,
        float trackSize,
        const GFXColor &color,
        bool drawArea) {
    if (drawArea) {
        GFXColor areaColor = color;
        areaColor.a /= 4;
        impl->areas.insert(GFXColorVertex(head, areaColor));
        impl->areas.insert(GFXColorVertex(ground, areaColor));
        impl->areas.insert(GFXColorVertex(center, areaColor));
    }

    // Diamond
    float size = 6.0 * std::max(trackSize, 1.0f);
    float xsize = size / configuration()->graphics.resolution_x;
    float ysize = size / configuration()->graphics.resolution_y;


    // Don't overflow the index type
    Impl::ElementBuffer::value_type base_index = Impl::ElementBuffer::value_type(impl->diamonds.size());
    if (base_index < (std::numeric_limits<Impl::ElementBuffer::value_type>::max() - 5)) {
        impl->diamonds.insert(head.x - xsize, head.y, 0.0f, color);
        impl->diamonds.insert(head.x, head.y - ysize, 0.0f, color);
        impl->diamonds.insert(head.x + xsize, head.y, 0.0f, color);
        impl->diamonds.insert(head.x, head.y + ysize, 0.0f, color);
        impl->diamonds.insert(head.x - xsize, head.y, 0.0f, color);
        impl->diamondIndices.push_back(base_index + 0);
        impl->diamondIndices.push_back(base_index + 1);
        impl->diamondIndices.push_back(base_index + 1);
        impl->diamondIndices.push_back(base_index + 2);
        impl->diamondIndices.push_back(base_index + 2);
        impl->diamondIndices.push_back(base_index + 3);
        impl->diamondIndices.push_back(base_index + 3);
        impl->diamondIndices.push_back(base_index + 4);
    }
}

} // namespace Radar

/*
 * navmap.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: hellcatv, ace123, surfdargent, klaussfreire, jacks, pyramid3d
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Evert Vorster
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

#include "navmap.h"

#include <algorithm>
#include <cmath>

/// Longest camera-to-target distance allowed when framing. A sector spans ~1e11
/// units between clusters and the galaxy is vaster still, so the whole thing has
/// to fit inside this.
static const double kMaxCameraDistance = 1e15;

/// The camera is kept this far off the poles, where the view direction lines up with
/// the world's up axis and the camera's right axis stops being defined.
static const double kPoleEpsilon = 0.05;

/// Pi and half pi, named here rather than using M_PI / M_PI_2: M_PI_2 is a glibc
/// extension that MSVC does not define, and M_PI only arrives via _USE_MATH_DEFINES.
static const double kPi = 3.14159265358979323846;
static const double kPiHalf = 1.57079632679489661923;
void NavMap::setCamera(float yaw, float pitch) {
    yaw_ = yaw;
    pitch_ = pitch;
}

void NavMap::setFov(float fov_rad) {
    // A field of view of zero would divide by zero when framing.
    fov_ = (fov_rad > 0.01f) ? fov_rad : 1.5708f;
}

QVector NavMap::forward() const {
    const double cos_pitch = std::cos(pitch_);
    const double cos_yaw = std::cos(yaw_);
    return QVector(cos_pitch * cos_yaw, std::sin(pitch_), -cos_pitch * std::sin(yaw_));
}

void NavMap::computeBasis(QVector &forward, QVector &right, QVector &up) const {
    forward = this->forward();

    const QVector world_up(0.0, 1.0, 0.0);
    right = forward.Cross(world_up);
    right.Normalize();
    up = right.Cross(forward);
    up.Normalize();
}

void NavMap::setFraming(const QVector &center, double halfx, double halfy, double halfz) {
    // Fit the widest extent rather than the corner-to-corner diagonal: the content of
    // a nav map is nearly planar, and fitting the diagonal stands the camera far enough
    // back that the map fills only about a third of the view.
    const double half_extent = std::max(halfx, std::max(halfy, halfz));
    const double tan_half_fov = std::tan(0.5 * fov_);

    double distance = half_extent / tan_half_fov;
    if (distance < 1.0) {
        distance = 1.0;
    }
    if (distance > kMaxCameraDistance) {
        distance = kMaxCameraDistance;
    }
    nom_dist_ = static_cast<float>(distance);

    QVector forward;
    QVector right;
    QVector up;
    computeBasis(forward, right, up);
    pos_ = center - (forward * distance);
}

void NavMap::orbitBy(float dyaw, float dpitch) {
    yaw_ += dyaw;
    pitch_ += dpitch;
    // Held just short of the poles, where the view direction would line up with the
    // world's up axis.
    if (pitch_ > (kPiHalf - kPoleEpsilon)) {
        pitch_ = static_cast<float>(kPiHalf - kPoleEpsilon);
    }
    if (pitch_ < (-kPiHalf + kPoleEpsilon)) {
        pitch_ = static_cast<float>(-kPiHalf + kPoleEpsilon);
    }
    // The yaw is still wrapped, only to keep it accurate over long drags; it is periodic,
    // so nothing else changes.
    if (yaw_ > kPi) {
        yaw_ -= static_cast<float>(2.0 * kPi);
    }
    if (yaw_ < -kPi) {
        yaw_ += static_cast<float>(2.0 * kPi);
    }
}

void NavMap::orbitAround(const QVector &pivot, float dyaw, float dpitch) {
    // A rigid turn of the whole camera about the pivot: the camera's position and its view
    // direction turn together, so whatever is at the pivot keeps the screen position it
    // already had while everything else swings around it. Turning the position and then
    // re-aiming at the pivot instead would drag the pivot to the middle of the screen.
    QVector old_forward;
    QVector old_right;
    QVector old_up;
    computeBasis(old_forward, old_right, old_up);

    orbitBy(dyaw, dpitch);

    QVector new_forward;
    QVector new_right;
    QVector new_up;
    computeBasis(new_forward, new_right, new_up);

    // Turn the offset from the pivot by the same rotation that turned the view.
    const QVector offset = pos_ - pivot;
    const QVector turned = (new_right * offset.Dot(old_right))
            + (new_up * offset.Dot(old_up))
            + (new_forward * offset.Dot(old_forward));
    pos_ = pivot + turned;
}

QVector NavMap::focusPoint() const {
    return pos_ + (forward() * nom_dist_);
}

void NavMap::panBy(float dright, float dup) {
    // Strafe along the camera's own axes: a pure translation, so it never re-aims
    // the view.
    QVector forward;
    QVector right;
    QVector up;
    computeBasis(forward, right, up);
    pos_ += right * dright;
    pos_ += up * dup;
}

void NavMap::zoomBy(float dist) {
    // Along the view direction. The caller scales dist by the distance to whatever
    // is nearest, so that zooming is quick across empty space and fine near things.
    QVector forward;
    QVector right;
    QVector up;
    computeBasis(forward, right, up);
    pos_ += forward * dist;
}

bool NavMap::project(const QVector &world, float &sx, float &sy, float &sscale) const {
    QVector forward;
    QVector right;
    QVector up;
    computeBasis(forward, right, up);

    const QVector to_point = world - pos_;
    const double along = to_point.Dot(forward);
    if (along <= 0.01) {
        return false;      //behind the camera, so nothing to draw
    }

    // Perspective projection: whatever is at the distance the camera was framed for
    // covers the view exactly, so 1.0 is the edge of the screen.
    const double focal = 1.0 / std::tan(0.5 * fov_);
    sx = static_cast<float>((to_point.Dot(right) / along) * focal);
    sy = static_cast<float>((to_point.Dot(up) / along) * focal);
    sscale = static_cast<float>(nom_dist_ / along);

    return true;
}

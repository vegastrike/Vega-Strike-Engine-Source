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

/// The camera is kept this far off the poles, where yaw stops meaning anything.
static const double kPoleEpsilon = 0.05;

void NavMap::setCamera(float yaw, float pitch) {
    yaw_ = yaw;
    pitch_ = pitch;
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

void NavMap::setFraming(const QVector &center, double halfx, double halfy, double halfz, float fov_rad) {
    // Fit the widest extent rather than the corner-to-corner diagonal: the content of
    // a nav map is nearly planar, and fitting the diagonal stands the camera far
    // enough back that the map fills only about a third of the view.
    //
    // project() hands back a tangent, which the caller reads as a coordinate where 1.0
    // is the edge of the view, so the field of view is the angle subtended by that
    // edge: at 90 degrees, an extent at the framing distance lands exactly on it.
    const double half_extent = std::max(halfx, std::max(halfy, halfz));
    const double tan_half_fov = std::tan(0.5 * (fov_rad > 0.01f ? fov_rad : 1.0f));

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
    if (pitch_ > (M_PI_2 - kPoleEpsilon)) {
        pitch_ = static_cast<float>(M_PI_2 - kPoleEpsilon);
    }
    if (pitch_ < (-M_PI_2 + kPoleEpsilon)) {
        pitch_ = static_cast<float>(-M_PI_2 + kPoleEpsilon);
    }
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

    // Perspective: divide by the distance ahead, scaled so that an extent framed at
    // the nominal distance covers the view.
    const double focal = nom_dist_;
    const double scale = focal / along;
    sx = static_cast<float>((to_point.Dot(right) * scale) / focal);
    sy = static_cast<float>((to_point.Dot(up) * scale) / focal);
    sscale = static_cast<float>(scale);

    return true;
}

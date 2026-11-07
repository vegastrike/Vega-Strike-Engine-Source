/*
 * navmap.h
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

#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_NAVMAP_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_NAVMAP_H

#include "gfx_generic/vec.h"

/// A free-fly camera for the nav map.
///
/// The camera holds a position and an orientation (yaw and pitch). A world point is
/// placed in the camera's frame and projected to a screen position, with a size
/// factor so that items further away are drawn smaller. At the nominal distance the
/// position is in the -0.5..0.5 range, which is what the nav item drawing expects.
///
/// The maths is done in QVector (double) because a sector can span ~1e11 units, and
/// float does not carry the relative offsets precisely enough to plot them.
class NavMap {
public:
    NavMap() = default;

    void setCamera(float yaw, float pitch);

    /// The vertical field of view in radians, from the game's own viewing angle so that
    /// the map has the same perspective as the world. It stays fixed: moving closer or
    /// further is a move of the camera, not a change of lens.
    void setFov(float fov_rad);

    /// Aim at center, with an extent of the given half-widths fitted to the view.
    void setFraming(const QVector &center, double halfx, double halfy, double halfz);

    /// Project a world point into a screen position. Returns false if the point is
    /// behind the camera. The position is in the -0.5..0.5 range at the nominal
    /// distance, and sscale is the perspective size factor, 1.0 at that distance.
    bool project(const QVector &world, float &sx, float &sy, float &sscale) const;

    /// Look around, staying where the camera is. Not to be confused with panBy().
    void orbitBy(float dyaw, float dpitch);

    /// Swing the camera around pivot, keeping it aimed at pivot and at the same
    /// distance from it, so whatever is at the pivot stays put while everything else
    /// moves around it. Used to circle the selected target.
    void orbitAround(const QVector &pivot, float dyaw, float dpitch);

    /// Move sideways, without changing the direction the camera looks in.
    void panBy(float dright, float dup);

    /// Move along the view direction. Positive is forward, so it zooms in.
    void zoomBy(float dist);

    const QVector &position() const {
        return pos_;
    }

    /// The direction the camera looks in.
    QVector forward() const;

    /// The point the camera is looking at, at the framing distance.
    QVector focusPoint() const;

    float nominalDistance() const {
        return nom_dist_;
    }

private:
    /// Fill in the view direction, and the right and up axes that go with it.
    void computeBasis(QVector &forward, QVector &right, QVector &up) const;

    QVector pos_{0.0, 0.0, -120.0};     // a little in front of the origin
    float yaw_{-0.6f};
    float pitch_{0.35f};
    float nom_dist_{120.0f};
    float fov_{1.5708f};                //radians; a quarter turn until the game sets it
};

#endif//VEGA_STRIKE_ENGINE_GFX_NAV_NAVMAP_H

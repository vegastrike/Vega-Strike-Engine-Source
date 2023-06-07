// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * cockpit_gfx_utils.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_COCKPIT_GFX_UTILS_H
#define VEGA_STRIKE_ENGINE_GFX_COCKPIT_GFX_UTILS_H

#include "vec.h"
#include "gfxlib_struct.h"

enum class ShapeType {
    Box,
    Cross,
    Diamond,
    Default
};

VertexBuilder<> GetCross(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& size);
VertexBuilder<> GetDiamond(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size);
VertexBuilder<> GetRectangle(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size);
VertexBuilder<> GetOpenRectangle(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size);
VertexBuilder<> GetLockingIcon(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const float& r_size,
                            const float& lock_percent);
VertexBuilder<> GetAnimatedLockingIcon(const QVector &location, const Vector& cam_p,
                           const Vector& cam_q, const Vector& cam_r,
                            const float& r_size,
                            const float& lock_percent);

void SetThickness(ShapeType type);

#endif //VEGA_STRIKE_ENGINE_GFX_COCKPIT_GFX_UTILS_H

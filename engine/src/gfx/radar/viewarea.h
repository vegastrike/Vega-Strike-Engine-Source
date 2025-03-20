// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/**
 * viewarea.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_RADAR_VIEWAREA_H
#define VEGA_STRIKE_ENGINE_GFX_RADAR_VIEWAREA_H

#include "vec.h"
#include "gfx/sprite.h"

struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

namespace Radar {

struct ViewArea {
    ViewArea();

    void SetSprite(VSSprite *);

    Vector Scale(const Vector &) const;

    GFXColor GetColor() const;

    bool IsActive() const;

    VSSprite *sprite;
    Vector position;
    Vector size;
};

} // namespace Radar

#endif //VEGA_STRIKE_ENGINE_GFX_RADAR_VIEWAREA_H

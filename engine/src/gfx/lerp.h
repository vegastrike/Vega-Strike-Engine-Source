/**
 * lerp.h
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


#ifndef _GFX_LERP_H_
#define _GFX_LERP_H_

#include <assert.h>
#include "vec.h"
#include "quaternion.h"

//Interpolates A and B, but does not cap the interpolation to less than 1.0
Transformation linear_interpolate_uncapped(const Transformation &a, const Transformation &b, double blend);

//Interpolates between the transformations A and B.  Caps the blending factor at 1.0, but not at 0.0
inline Transformation linear_interpolate(const Transformation &a, const Transformation &b, double blend) {
    return linear_interpolate_uncapped(a, b, ((blend > 1.0) ? 1.0 : blend));
}

#endif


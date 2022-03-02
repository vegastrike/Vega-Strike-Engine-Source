/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../../first.h"
#include "../fvector.h"

fvector::fvector(ccoords const &cc) {
    float inv_magnitude = 1.0f
            / ::sqrtf(cc.x_ * cc.x_ + cc.y_ * cc.y_ + cc.z_ * cc.z_);
    x_ = cc.x_ * inv_magnitude;
    y_ = cc.y_ * inv_magnitude;
    z_ = cc.z_ * inv_magnitude;
}

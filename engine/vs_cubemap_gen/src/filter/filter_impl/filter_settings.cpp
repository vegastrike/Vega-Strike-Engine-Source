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
#include "../filter_settings.h"
#include "filter_math.h"

void filter_settings::init()
{
    vs_assert(top_lod_size > ambient_lod_size, "lod sizes make no sense");
    vs_assert(is_POT(top_lod_size), "target texture size must be a power of two");
    vs_assert(is_POT(ambient_lod_size), "target ambient lod size must be a power of two");
    lod_ratio_ = double(top_lod_size) / double(ambient_lod_size);
    lod_steps_ = poweroftwo_steps_from_ratio(lod_ratio);
    min_flt_radius_ = texel_radius_from_side(max_texel_side_radians(top_lod_size));
    amb_flt_radius_ = ambient_flt_radius();
    flt_radius_ratio_ = amb_flt_radius_ / min_flt_radius_;
    angle_scaling_ = angle_scaling_factor(flt_radius_ratio_, lod_steps_);
}


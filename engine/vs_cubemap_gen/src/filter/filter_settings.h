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

#ifndef __FILTER_MATH_H__
#define __FILTER_MATH_H__

class filter_settings {
    //input params:
    size_t top_lod_size_;
    size_t ambient_lod_size_;
    //computed params:
    double lod_ratio_;
    size_t lod_steps_;
    radians min_flt_radius_;
    radians amb_flt_radius_;
    float flt_radius_ratio_;
    double angle_scaling_;
public:
    filter_settings(
            size_t top_lod_size        //e.g. 1024
            , size_t ambient_lod_size    //e.g.    8
    )
            : top_lod_size_(top_lod_size), ambient_lod_size_(ambient_lod_size)
    {
    }

    void init();
    void print();
};

#endif



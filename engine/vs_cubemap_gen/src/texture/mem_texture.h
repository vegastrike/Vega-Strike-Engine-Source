/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_CUBE_MAP_MEM_TEXTURE_H
#define VEGA_STRIKE_ENGINE_CUBE_MAP_MEM_TEXTURE_H

//this file presents:
class mem_texture;

//this file references:
class fcolor;
class steradians;

//it also defines:
typedef std::pair<fcolor, steradians> tex_fetch_t;

struct RectIterRanges {
    unsigned startu, endu, startv, endv;
};

template<size_t SIZE>
class mem_tex {
    float buff[6 * SIZE * SIZE * 4]; //6 square SIZE textures * RGBA -worth of floats
    size_t tex_index(side const &s, icoords const &ic) const;
    radians angles_from_center_[SIZE >> 1];
public:
    mem_tex();

    size_t size() const {
        return SIZE;
    }

    radians angle_from_center(size_t single_coord) const;
    radians texel_angle_width(size_t single_coord) const;
    steradians texel_solid_angle(icoords const &ic) const;
    fRGBAcolor const *pTexel(side const &s, icoords const &ic) const;
    fRGBAcolor *pTexel(side const &s, icoords const &ic);
    RectIterRanges get_iter_ranges(side_and_coords const &sc1, side_and_coords const &sc2);
};

#endif //VEGA_STRIKE_ENGINE_CUBE_MAP_MEM_TEXTURE_H

/**
* mem_texture.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#ifndef __MEM_TEXTURE_H__
#define __MEM_TEXTURE_H__

//this file presents:
class mem_texture;

//this file references:
class fcolor;
class steradians;

//it also defines:
typedef std::pair< fcolor, steradians > tex_fetch_t;

struct RectIterRanges
{
    unsigned startu, endu, startv, endv;
};

template < size_t SIZE >
class mem_tex
{
    float buff[6*SIZE*SIZE*4]; //6 square SIZE textures * RGBA -worth of floats
    size_t tex_index( side const & s, icoords const & ic ) const;
    radians angles_from_center_[SIZE>>1];
public:
    mem_tex();
    size_t size() const { return SIZE; }
    radians angle_from_center( size_t single_coord ) const;
    radians texel_angle_width( size_t single_coord ) const;
    steradians texel_solid_angle( icoords const & ic ) const;
    fRGBAcolor const * pTexel( side const & s, icoords const & ic ) const;
    fRGBAcolor       * pTexel( side const & s, icoords const & ic );
    RectIterRanges     get_iter_ranges( side_and_coords const & sc1, side_and_coords const & sc2 );
};


#endif



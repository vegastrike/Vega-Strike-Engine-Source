/**
 * mesh_io.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_MESH_IO_H
#define VEGA_STRIKE_ENGINE_GFX_MESH_IO_H

#define uint32bit unsigned int
#define float32bit float
#define uchar8bit unsigned char

#ifdef DEFINE_BLENDFUNC
enum BLENDFUNC {
    ZERO = 1,
    ONE = 2,
    SRCCOLOR = 3,
    INVSRCCOLOR = 4,
    SRCALPHA = 5,
    INVSRCALPHA = 6,
    DESTALPHA = 7,
    INVDESTALPHA = 8,
    DESTCOLOR = 9,
    INVDESTCOLOR = 10,
    SRCALPHASAT = 11,
    CONSTALPHA = 12,
    INVCONSTALPHA = 13,
    CONSTCOLOR = 14,
    INVCONSTCOLOR = 15
};
#endif

struct Mesh_vec3f {
    float x;
    float y;
    float z;
};
enum textype {
    ALPHAMAP,
    ANIMATION,
    TEXTURE,
    TECHNIQUE
};

#endif //VEGA_STRIKE_ENGINE_GFX_MESH_IO_H

/*
 * filter.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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

#include "../filter.h"

void filter::init_constants() {
}

void filter::do_it() {
    filter_one_texel flt_one_tex(source_, target_, shininess_);
    fvector v;
    for (size_t s = eLeft; s < eBack; ++s) {
        for (size_t i = 0; i < target.size(); ++i) {
            for (size_t j = 0; j < target.size(); ++j) {
                mem_texture &tmt = target.get_buffer();
                icoords ic(i, j);
                fRGBAcolor *pt = tmt.pTexel(s, ic);
                ccoords cc = ccoords(side_and_coords(s, scoords(ucoords(ic))));
                v = fvector(cc);
                *pt = flt_one_tex(v);
            }
        }
    }
}

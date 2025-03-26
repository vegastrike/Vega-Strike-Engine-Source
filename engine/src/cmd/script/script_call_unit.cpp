/*
 * script_call_unit.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>

//This takes care of the fact that several systems use the _POSIX_C_SOURCE
//variable and don't set them to the same thing.
//Python.h sets and uses it
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif //_POSIX_C_SOURCE

#include <Python.h>
#include <string>

#include "gfx/aux_texture.h"
#include "gfx/cockpit.h"
#include "gfx/animation.h"
#include "cmd/planet.h"

bool PlanetHasLights(Unit *un) {
    return ((Planet *) un)->hasLights();
}


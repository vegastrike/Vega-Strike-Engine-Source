/*
 * unit_method_defs.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#if _MSC_VER <= 1'200
#    define PY_SSIZE_T_CLEAN
#    include <boost/python.hpp>

#    include <boost/version.hpp>
typedef boost::python::dict BoostPythonDictionary;

#    include <string>

#    include "cmd/ai/fire.h"
#    include "cmd/container.h"
#    include "cmd/unit_generic.h"
#    include "cmd/unit_util.h"
#    include "gfx_generic/vec.h"
#    include "root_generic/faction_generic.h"
#    include "src/python/define_odd_unit_functions.h"
#    include "src/python/init.h"
#    include "src/python/python_class.h"
#    include "src/python/unit_from_to_python.h"
#    include "src/python/unit_wrapper_class.h"
#    include "src/universe_util.h"
#endif

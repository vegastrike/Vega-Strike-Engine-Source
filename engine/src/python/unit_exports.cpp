/*
 * unit_exports.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python.hpp>
typedef boost::python::dict BoostPythonDictionary;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary;
#endif

#include "python_class.h"

#include "cmd/container.h"
#include <string>
#include "init.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"

#include "universe_util.h"
#include "cmd/unit_util.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"

#include "unit_wrapper_class.h"
#include "unit_from_to_python.h"
extern void ExportUnitFIRST(class boost::python::class_builder<class UnitWrapper> &Class);
extern void ExportUnitSECOND(boost::python::class_builder<UnitWrapper> &Class);
extern void ExportUnitTHIRD(boost::python::class_builder<UnitWrapper> &Class);

void ExportUnit(boost::python::class_builder<UnitWrapper> &Class) {
    ExportUnitFIRST(Class);
    ExportUnitSECOND(Class);
    ExportUnitTHIRD(Class);
}


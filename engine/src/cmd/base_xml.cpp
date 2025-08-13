/*
 * base_xml.cpp
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


#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#include <boost/python.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include <Python.h>
#undef _CRT_NONSTDC_NO_DEPRECATE
#include <vector>
#include <string>
#include <math.h>
#include "src/python/python_class.h"
#include "cmd/base.h"
#include "cmd/base_util.h"
#include "root_generic/vsfilesystem.h"

static FILE *withAndWithout(std::string filename, std::string time_of_day_hint) {
    string with(filename + "_" + time_of_day_hint + BASE_EXTENSION);
    FILE *fp = VSFileSystem::vs_open(with.c_str(), "r");
    if (!fp) {
        string without(filename + BASE_EXTENSION);
        fp = VSFileSystem::vs_open(without.c_str(), "r");
    }
    return fp;
}

static FILE *getFullFile(std::string filename, std::string time_of_day_hint, std::string faction) {
    FILE *fp = withAndWithout(filename + "_" + faction, time_of_day_hint);
    if (!fp) {
        fp = withAndWithout(filename, time_of_day_hint);
    }
    return fp;
}

void BaseInterface::Load(const char *filename, const char *time_of_day_hint, const char *faction) {
    FILE *inFile = getFullFile(string("bases/") + filename, time_of_day_hint, faction);
    if (!inFile) {
        bool planet = false;
        Unit *baseun = this->baseun.GetUnit();
        if (baseun) {
            planet = (baseun->getUnitType() == Vega_UnitType::planet);
        }
        string basestring("bases/unit");
        if (planet) {
            basestring = "bases/planet";
        }
        inFile = getFullFile(basestring, time_of_day_hint, faction);
        if (!inFile) {
            return;
        }
    }
    //now that we have a FILE * named inFile and a std::string named newfile we can finally begin the python
    string compilefile = string(filename) + time_of_day_hint + string(faction) + BASE_EXTENSION;
    Python::reseterrors();
    VegaPyRunFile(inFile, compilefile);
    Python::reseterrors();
    VSFileSystem::vs_close(inFile);
}


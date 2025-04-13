/*
 * pythonmission.cpp
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
#include <Python.h>
#include <math.h>
#include "pythonmission.h"
#include "python/python_compile.h"
#include <string>
#include "python/init.h"

#include "vs_logging.h"
#include "cmd/container.h"

PythonMissionBaseClass::PythonMissionBaseClass() {
}

void PythonMissionBaseClass::Destructor() {
    delete this;
}

PythonMissionBaseClass::~PythonMissionBaseClass() {
    for (unsigned int i = 0; i < relevant_units.size(); ++i) {
        relevant_units[i]->SetUnit(NULL);
        delete relevant_units[i];
    }
    relevant_units.clear();
    VS_LOG(warning, (boost::format("BASE Destruct called. If called from C++ this is death %x") % this));
}

void PythonMissionBaseClass::Execute() {
}

void PythonMissionBaseClass::callFunction(std::string) {
}

std::string PythonMissionBaseClass::Pickle() {
    return std::string();
}

void PythonMissionBaseClass::UnPickle(std::string s) {
}


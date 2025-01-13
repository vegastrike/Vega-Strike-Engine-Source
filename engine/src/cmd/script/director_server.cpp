/*
 * director_server.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#define PY_SSIZE_T_CLEAN
#include "python/python_class.h"
#include <boost/version.hpp>
#include <boost/python.hpp>
#include <boost/python/class.hpp>
#ifdef HAVE_PYTHON
#include <Python.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif

#include "cmd/unit_generic.h"
#include "mission.h"

PYTHON_BEGIN_MODULE(Base)
//Nothing here, but keeps those files that do a "import Base" happy.
PYTHON_END_MODULE(Base)

PYTHON_BEGIN_MODULE(Briefing)
//Nothing here, but keeps those files that do a "import Briefing" happy.
PYTHON_END_MODULE(Briefing)

void InitBase() {
    PyImport_AppendInittab("Base", PYTHON_MODULE_INIT_FUNCTION(Base));
}

void InitBriefing() {
    PyImport_AppendInittab("Briefing", PYTHON_MODULE_INIT_FUNCTION(Briefing));
}

void InitBase2() {
    Python::reseterrors();
    PYTHON_INIT_MODULE(Base);
    Python::reseterrors();
}

void InitBriefing2() {
    Python::reseterrors();
    PYTHON_INIT_MODULE(Briefing);
    Python::reseterrors();
}

void Mission::DirectorLoop() {
    gametime += SIMULATION_ATOM;     //elapsed;

    try {
        if (runtime.pymissions) {
            runtime.pymissions->Execute();
        }
    }
    catch (...) {
        if (PyErr_Occurred()) {
            PyErr_Print();
            PyErr_Clear();
            fflush(stderr);
            fflush(stdout);
        }
        throw;
    }
}

void Mission::BriefingUpdate() {
}

void Mission::DirectorBenchmark() {
}

varInst *Mission::call_briefing(missionNode *node, int mode) {
    return NULL;
}


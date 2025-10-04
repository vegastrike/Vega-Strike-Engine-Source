/*
 * vega_py_run.cpp
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
#include <boost/python.hpp>

#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include "cmd/vega_py_run.h"
#include "vega_string_utils.h"
#include "src/vs_logging.h"

void VegaPyRunString(const std::string &py_snippet) {
    char * py_snippet_temp = vega_str_dup(py_snippet.c_str());
    VegaPyRunString(py_snippet_temp);
    free(py_snippet_temp);
}

void VegaPyRunString(char *py_snippet) {
    VS_LOG(trace, (boost::format("running %1%") % py_snippet));
    PyRun_SimpleString(py_snippet);
    if (PyErr_Occurred()) {
        VS_LOG_AND_FLUSH(error, "VegaPyRunString: Python error occurred");
        PyErr_Print();
        PyErr_Clear();
        VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    }
}

void VegaPyRunFile(FILE* fp, std::string file_path_or_description) {
    VS_LOG(trace, (boost::format("running file '%1%'") % file_path_or_description));
    PyRun_SimpleFile(fp, file_path_or_description.c_str());
    if (PyErr_Occurred()) {
        VS_LOG_AND_FLUSH(error, "VegaPyRunFile: Python error occurred");
        PyErr_Print();
        PyErr_Clear();
    }
}

/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include "vega_py_run.hpp"
#include "vs_logging.h"

void VegaPyRunString(const std::string &py_snippet) {
    VS_LOG(info, (boost::format("running %1%") % py_snippet));
    char * py_snippet_temp = strdup(py_snippet.c_str());
    if (!py_snippet_temp) {
        VS_LOG_AND_FLUSH(error, "VegaPyRunString: strdup failed");
        return;
    }
    VegaPyRunString(py_snippet_temp);
    free(py_snippet_temp);
}

void VegaPyRunString(char *py_snippet) {
#if defined(_WINDOWS) && defined(_DEBUG)
    VS_LOG(info, (boost::format("Debug mode on Windows; not running %1%") % py_snippet));
#else
    VS_LOG(info, (boost::format("running %1%") % py_snippet));
    PyRun_SimpleString(py_snippet);
    //Python::reseterrors();
    if (PyErr_Occurred()) {
        VS_LOG_AND_FLUSH(error, "VegaPyRunString: Python error occurred");
        PyErr_Print();
        PyErr_Clear();
        VegaStrikeLogging::vega_logger()->FlushLogs();
    }
#endif
}

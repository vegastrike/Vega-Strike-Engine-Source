/*
 * pythonai.cpp
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


#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#include <boost/python.hpp>
#include <boost/python/class.hpp>

#include <Python.h>
#include <compile.h>
#if ((PY_VERSION_HEX) < 0x030B0000)
#include <eval.h>
#endif
#include <stdio.h>

#include "python/python_class.h"
#include "python/python_compile.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "pythonai.h"

using namespace Orders;

PythonAI *PythonAI::last_ai = NULL;

PythonAI::PythonAI(PyObject *self_, float reaction_time, float aggressivity) : FireAt(reaction_time, aggressivity) {
    self = self_;
    //boost::python:
    Py_XINCREF(self);     //by passing this to whoami, we are counting on them to Destruct us
    last_ai = this;
}

void PythonAI::Destruct() {
    Py_XDECREF(self);     //this should destroy SELF
}

void PythonAI::default_Execute(FireAt &self_) {
    (self_).FireAt::Execute();
}

PythonAI *PythonAI::LastAI() {
    PythonAI *myai = last_ai;
    last_ai = NULL;
    return myai;
}

PythonAI *PythonAI::Factory(const std::string &filename) {
    CompileRunPython(filename);
    return LastAI();
}

void PythonAI::Execute() {
    boost::python::callback<void>::call_method(self, "Execute");
}

void PythonAI::InitModuleAI() {
    boost::python::module_builder ai_builder("AI");
    boost::python::class_builder<FireAt, PythonAI> BaseClass(ai_builder, "FireAt");

    BaseClass.def(boost::python::constructor<float, float>());
    BaseClass.def(&FireAt::Execute, "PythonAI", PythonAI::default_Execute);
}

PythonAI::~PythonAI() {
    VS_LOG(warning, (boost::format("Destruct called. If called from C++ this is death %1$x") % this));
}


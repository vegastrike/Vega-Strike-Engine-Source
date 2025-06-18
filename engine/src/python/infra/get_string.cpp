/*
 * get_string.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "get_string.h"
#include "root_generic/vsfilesystem.h"

#include <Python.h>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>



const std::string GetString(const std::string function_name,
                            const std::string module_name,
                            const std::string file_name,
                            PyObject* args) {
    PyObject* module = PyImport_ImportModule(module_name.c_str());

    if(!module) {
        VS_LOG_AND_FLUSH(error, "Error: PyImport_ImportModule failed");
        PyErr_Print();
        PyErr_Clear();
        return "Error: PyImport_ImportModule is null";
    }

    PyObject* function = PyObject_GetAttrString(module, function_name.c_str());
    if(!function) {
        VS_LOG_AND_FLUSH(error, "Error: PyObject_GetAttrString failed");
        PyErr_Print();
        PyErr_Clear();
        return "Error: PyObject_GetAttrString is null";
    }

    if(args == nullptr) {
        VS_LOG_AND_FLUSH(error, "Error: args is null");
        PyErr_Print();
        PyErr_Clear();
        return "Error: PyTuple_Pack is null";
    }

    PyObject* pyResult = PyObject_CallObject(function, args);

    if(!pyResult) {
        VS_LOG_AND_FLUSH(error, "Error: PyObject_CallObject failed");
        PyErr_Print();
        PyErr_Clear();
        return "Error: PyObject_CallObject is null";
    }

    std::string result = PyUnicode_AsUTF8(pyResult);

    return result;
}


const std::string GetString(const std::string function_name,
                            const std::string module_name,
                            const std::string file_name,
                            const std::map<std::string, std::string>& cpp_map) {
    boost::python::dict dict;
    for (auto const& pair : cpp_map) {
        dict[pair.first] = pair.second;
    }

    PyObject* args = PyTuple_Pack(1, dict.ptr());

    return GetString(function_name, module_name, file_name, args);
}

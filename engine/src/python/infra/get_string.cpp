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
#include "vegadisk/vsfilesystem.h"

#include <Python.h>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

/** Converts PyObject* raw pointer to RAII managed wrapper. */
boost::python::object WrapObject(PyObject* raw_object) {
    if(!raw_object) {
        VS_LOG_AND_FLUSH(error, "Error: WrapObject raw_object is null");
        PyErr_Print();
        PyErr_Clear();
        throw std::runtime_error("Error: WrapObject raw_object is null");
    }
    
    // Transfer ownership to RAII wrapper
    return boost::python::object{boost::python::handle<>(raw_object)};
}

boost::python::object GetPyObject(const std::string& function_name,
                                  const std::string& module_name,
                                  const std::string& file_name,
                                  PyObject* raw_args) {
    boost::python::object module = WrapObject(PyImport_ImportModule(module_name.c_str()));
    boost::python::object function = WrapObject(PyObject_GetAttrString(module.ptr(), function_name.c_str()));
    boost::python::object args = WrapObject(raw_args);
    boost::python::object result = WrapObject(PyObject_CallObject(function.ptr(), args.ptr()));
    
    return result;
}

std::string GetString(const std::string& function_name,
                     const std::string& module_name,
                     const std::string& file_name,
                     PyObject* raw_args) {
    boost::python::object result = GetPyObject(function_name, module_name, file_name, raw_args);
    
    const char* c_str = PyUnicode_AsUTF8(result.ptr());
    if (!c_str) {
        VS_LOG_AND_FLUSH(error, "Error: Failed to convert Python object to UTF-8 std::string in GetString");
        PyErr_Print();
        PyErr_Clear();
        throw std::runtime_error("Error: Failed to convert Python object to UTF-8 std::string in GetString");
    }
    
    return std::string(c_str);
}

boost::python::object MapToObject(const std::map<std::string, std::string>& cpp_map) {
    boost::python::dict dict;
    for (const auto& pair : cpp_map) {
        dict[pair.first] = pair.second;
    }
    
    return WrapObject(PyTuple_Pack(1, dict.ptr()));
}

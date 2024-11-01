/*
 * ship_view.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "ship_view.h"
#include "vsfilesystem.h"

#include <Python.h> 
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

static const std::string module_name = "ship_view";

const std::string GetShipView(const std::map<std::string, std::string>&ship_stats) {
    if(!boost::filesystem::exists("python/base_computer/ship_view.py")) {
        return "Error: ship description not found";
    }
    
    PyObject* module = PyImport_ImportModule(module_name.c_str());

    if(!module) {
        PyErr_Print();
        return "Error: PyImport_ImportModule is null";
    }

    boost::python::dict dict;
    for (auto const& pair : ship_stats) {
        dict[pair.first] = pair.second;
    }


    PyObject* args = PyTuple_Pack(1, dict.ptr());
    if(args == nullptr) {
        PyErr_Print();
        return "Error: PyTuple_Pack is null";
    } 

    PyObject* function = PyObject_GetAttrString(module,"get_ship_description");
    if(!function) {
        PyErr_Print();
        return "Error: PyObject_GetAttrString is null";
    } 
    
    
    PyObject* pyResult = PyObject_CallObject(function, args);

    if(!pyResult) {
        PyErr_Print();
        return "Error: PyObject_CallObject is null";
    }

    std::string result = PyUnicode_AsUTF8(pyResult);

    return result;
}
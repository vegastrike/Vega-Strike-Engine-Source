/*
 * python_utils.cpp
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

#include "python_utils.h"

#include <iostream>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

using namespace boost::python;
using namespace boost::filesystem;

#define Q(x) #x
#define QUOTE(x) Q(x)

// This is a kludge. It runs python before
// just to get the python paths.
const std::string GetPythonPath() {
    Py_Initialize();
    wchar_t* w_path_ptr = Py_GetPath();
    Py_Finalize();
    
    std::wstring w_path_w( w_path_ptr );
    std::string path( w_path_w.begin(), w_path_w.end() );

    return path;
}



// A utility function to call a function and get a PyObject as a result
PyObject* GetClassFromPython(
    const std::string build_path,
    const std::string path_string,
    const std::string module_name,
    const std::string function_name) {
    boost::filesystem::path full_path = boost::filesystem::path(path_string);
    boost::filesystem::current_path(full_path);

    if(!boost::filesystem::exists(module_name + ".py")) {
        // TODO: throw exception
        return nullptr;
    }
    
    const std::string yaml_path = QUOTE(Python_SITELIB);
    const std::string python_path_string = GetPythonPath() 
                                           + ":" + path_string
                                           + ":" + yaml_path + ":" + build_path;
    const std::wstring python_path_wstring = std::wstring(python_path_string.begin(), 
                                                          python_path_string.end());
    const wchar_t* python_path = python_path_wstring.c_str();
    Py_SetPath(python_path);
    Py_Initialize();

    
    PyObject* module = PyImport_ImportModule(module_name.c_str());

    if(!module) {
        std::cerr << "PyImport_ImportModule is null\n" << std::flush;
        PyErr_Print();
        Py_Finalize();
        // TODO: throw exception
        return nullptr;
    } 

    PyObject* function = PyObject_GetAttrString(module,function_name.c_str());
    if(!function) {
        std::cerr << "PyObject_GetAttrString is null\n" << std::flush;
        PyErr_Print();
        Py_Finalize();
        // TODO: throw exception
        return nullptr;
    } 
    
    
    PyObject* pyResult = PyObject_CallObject(function, nullptr);
    Py_Finalize();

    return pyResult;
}






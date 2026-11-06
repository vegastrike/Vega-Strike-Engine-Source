/*
 * python_utils.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "python_utils.h"

#include <iostream>
#include <list>
#include <string>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

//#include "vegadisk/vsfilesystem.h"
//#include "src/vs_logging.h"

using namespace boost::python;
using namespace boost::filesystem;

#define Q(x) #x
#define QUOTE(x) Q(x)

// This is a kludge. It runs python before
// just to get the python paths.
std::string GetPythonPath() {
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
    const std::string datadir,
    const std::string module_name,
    const std::string function_name) {
    boost::filesystem::path full_path = boost::filesystem::path(path_string);
    boost::filesystem::current_path(full_path);

    if(!boost::filesystem::exists(module_name + ".py")) {
        // TODO: throw exception
        return nullptr;
    }

    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);

    py_pre_config.coerce_c_locale = 1;

    PyStatus status;

    status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status)) {
        std::cerr << "Python pre-initialization failed" << std::endl << std::flush;
        Py_ExitStatusException(status);
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.isolated = 0;

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config); // Free configuration memory
    if (PyStatus_Exception(status)) {
        std::cerr << "Python config initialization failed" << std::endl << std::flush;
        PyErr_Print();
        Py_ExitStatusException(status);
    }

    const std::wstring path_string_w = std::wstring(path_string.begin(), path_string.end());
    const std::string python_sitelib_path = QUOTE(Python_SITELIB);
    const std::wstring python_sitelib_path_wstring = std::wstring(python_sitelib_path.begin(), python_sitelib_path.end());
    const std::wstring build_path_w = std::wstring(build_path.begin(), build_path.end());
    const std::wstring data_path_w = std::wstring(datadir.begin(), datadir.end());

    std::list<std::wstring> const path_wstrs{path_string_w, python_sitelib_path_wstring, build_path_w, data_path_w};
    std::list<boost::filesystem::path> paths;
    for (const auto& path : path_wstrs) {
        boost::filesystem::path const canonical_path = boost::filesystem::canonical(path);
        boost::filesystem::path const lexically_normal = canonical_path.lexically_normal();
        paths.push_back(lexically_normal);
    }

    PyObject* sys_path = PySys_GetObject("path");
    if (sys_path == nullptr) {
        std::cerr << boost::format("%1%: %2%: PySys_GetObject(\"path\") returned nullptr")
                % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    for (const auto& search_path : paths) {
        boost::filesystem::path const canonical_path = boost::filesystem::canonical(search_path);
        std::string const canonical_path_str = canonical_path.lexically_normal().generic_string();
        std::wstring const canonical_path_wstr = canonical_path.lexically_normal().generic_wstring();
        PyObject* path_element = PyUnicode_FromWideChar(canonical_path_wstr.c_str(), canonical_path_wstr.length());
        if (path_element == nullptr) {
            std::cerr << boost::format("%1%: %2%: PyUnicode_FromWideChar returned nullptr")
                    % __FILE__ % __FUNCTION__ << std::endl;
            return nullptr;
        }
        int const append_status = PyList_Append(sys_path, path_element);
        Py_DECREF(path_element); // Clean up temporary reference allocation
        if (append_status != 0) {
            std::cerr << boost::format("%1%: %2%: PyList_Append returned %3% for path %4%")
                    % __FILE__ % __FUNCTION__ % append_status % canonical_path_str << std::endl;
        }
    }

    PyObject* module = PyImport_ImportModule(module_name.c_str());

    if(!module) {
        std::cerr << "Python module import failed (PyImport_ImportModule is null)" << std::endl << std::flush;
        PyErr_Print();
        Py_Finalize();
        // TODO: throw exception
        return nullptr;
    }

    PyObject* function = PyObject_GetAttrString(module,function_name.c_str());
    if(!function) {
        std::cerr << "PyObject_GetAttrString is null" << std::endl << std::flush;
        PyErr_Print();
        Py_Finalize();
        // TODO: throw exception
        return nullptr;
    }


    PyObject* pyResult = PyObject_CallObject(function, nullptr);
    Py_Finalize();

    return pyResult;
}

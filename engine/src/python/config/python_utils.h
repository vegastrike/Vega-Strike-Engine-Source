/*
 * python_utils.h
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

#ifndef VEGA_STRIKE_ENGINE_SRC_PYTHON_CONFIG_PYTHON_UTILS_U
#define VEGA_STRIKE_ENGINE_SRC_PYTHON_CONFIG_PYTHON_UTILS_U

#include <list>
#include <string>

#include <boost/filesystem/path.hpp>

// See https://stackoverflow.com/q/804894/5276890
struct _object;
typedef _object PyObject;

std::string GetPythonPath();

/// Initialize Python if not already initialized
void InitPython(bool& was_already_initialized);

/// A utility function to call a Python function with no arguments and get a PyObject as a result
PyObject* GetClassFromPython(
    std::string build_path,
    std::wstring path_string,
    std::string datadir,
    std::string module_name,
    std::string function_name);

/// A utility function to call a Python function with no arguments and get a PyObject as a result. Takes arguments as
/// const references, which should be more efficient.
PyObject* GetClassFromPythonPathRefs(
        const std::string& build_path,
        const std::wstring &path_string,
        const std::string& datadir,
        const std::string& module_name,
        const std::string& function_name);

/// Get a pointer to a Python function/method
PyObject* GetMethodFromPython(std::string module_name,
    std::string function_name,
    const std::list<boost::filesystem::path>& search_paths);

/// Get a pointer to a Python function/method
PyObject* GetMethodFromPython(std::string module_name,
    std::string function_name,
    const std::list<std::string>& search_paths);

/// Get a pointer to a Python function/method
PyObject* GetMethodFromPython(std::string module_name,
    std::string function_name,
    const std::list<std::wstring>& search_paths);

/// Get a pointer to a Python function/method
PyObject* GetMethodFromPython(
    const std::string& build_path,
    const std::wstring& path_string,
    const std::string& datadir,
    const std::string& module_name,
    const std::string& function_name);

/// Pack the list of args provided into a Tuple to be passed to a Python function
PyObject* PrepareStringArgsForPython(const std::list<std::string>& args);

/// Pack the list of args provided into a Tuple to be passed to a Python function
PyObject* PrepareWStringArgsForPython(const std::list<std::wstring>& args);

/// Call a Python function/method with no arguments
PyObject* CallPythonFnNullArgs(std::string module_name,
    std::string function_name,
    const std::list<boost::filesystem::path>& search_paths);

PyObject *CallPythonFnNullArgs(PyObject *py_fn);

PyObject* CallPythonFnStringArgs(std::string module_name,
                                 std::string function_name,
                                 const std::list<boost::filesystem::path>& search_paths,
                                 const std::list<std::string>& args);

PyObject* CallPythonFnStringArgs(PyObject *py_fn, const std::list<std::string>& args);

PyObject* CallPythonFnWStringArgs(std::string module_name,
    std::string function_name,
    const std::list<boost::filesystem::path>& search_paths,
    const std::list<std::wstring>& args);

PyObject* CallPythonFnWStringArgs(PyObject *py_fn, const std::list<std::wstring>& args);

PyObject* CallPythonFnPyArgs(PyObject *py_fn, PyObject *args);

#endif //VEGA_STRIKE_ENGINE_SRC_PYTHON_CONFIG_PYTHON_UTILS_U

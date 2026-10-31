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

#define PY_SSIZE_T_CLEAN

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <iostream>
#include <utility>

#include "python_utils.h"

#include <boost/format.hpp>

using namespace boost::python;
using namespace boost::filesystem;

#define Q(x) #x
#define QUOTE(x) Q(x)

// This is a kludge. It runs python before
// just to get the python paths.
std::string GetPythonPath() {
    Py_Initialize();
    wchar_t const* w_path_ptr = Py_GetPath();
    Py_Finalize();

    std::wstring w_path_w( w_path_ptr );
    std::string path( w_path_w.begin(), w_path_w.end() );

    return path;
}

void InitPython(bool &was_already_initialized) {
    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);
    py_pre_config.coerce_c_locale = 1;
    was_already_initialized = Py_IsInitialized() != 0;
    if (was_already_initialized) {
        return;
    }

    PyStatus status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status) != 0) {
        PyErr_Print();
        Py_ExitStatusException(status);
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 0;

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config); // Free configuration memory
    if (PyStatus_Exception(status) != 0) {
        PyErr_Print();
        Py_ExitStatusException(status);
    }
}

PyObject * GetMethodFromPython(std::string module_name, std::string function_name
                               , const std::list<boost::filesystem::path>& search_paths) {
    PyObject* sys_path = PySys_GetObject("path");
    if (sys_path == nullptr) {
        std::cerr << boost::format("%1%: %2%: PySys_GetObject(\"path\") returned nullptr")
                % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    for (const auto& search_path : search_paths) {
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
    if (module == nullptr) {
        PyErr_Print();
        std::cerr << boost::format("%1%: %2%: PyImport_ImportModule() returned nullptr for module %3%")
                % __FILE__ % __FUNCTION__ % module_name << std::endl;
        return nullptr;
    }

    PyObject* function = PyObject_GetAttrString(module,function_name.c_str());
    if (function == nullptr) {
        PyErr_Print();
        std::cerr << boost::format("%1%: %2%: PyObject_GetAttrString returned nullptr for function %3%")
            % __FILE__ % __FUNCTION__ % function_name << std::endl;
        return nullptr;
    }

    return function;
}

PyObject * GetMethodFromPython(std::string module_name,
        std::string function_name,
        const std::list<std::string> &search_paths) {
    std::list<boost::filesystem::path> paths;
    for (const auto& path : search_paths) {
        boost::filesystem::path const canonical_path = boost::filesystem::canonical(path);
        boost::filesystem::path const lexically_normal = canonical_path.lexically_normal();
        paths.push_back(canonical_path);
    }

    return GetMethodFromPython(std::move(module_name), std::move(function_name), paths);
}

PyObject * GetMethodFromPython(std::string module_name,
        std::string function_name,
        const std::list<std::wstring> &search_paths) {
    std::list<boost::filesystem::path> paths;
    for (const auto& path : search_paths) {
        boost::filesystem::path const canonical_path = boost::filesystem::canonical(path);
        boost::filesystem::path const lexically_normal = canonical_path.lexically_normal();
        paths.push_back(canonical_path);
    }

    return GetMethodFromPython(std::move(module_name), std::move(function_name), paths);
}

PyObject * GetMethodFromPython(const std::string &build_path, const std::wstring &path_string
        , const std::string &datadir, const std::string &module_name, const std::string &function_name) {
    boost::filesystem::path const full_path = boost::filesystem::path(path_string);
    boost::filesystem::current_path(full_path);

    if(!boost::filesystem::exists(module_name + ".py")) {
        std::cerr << boost::format("%1%: %2%: Can't find module %3%.") % __FILE__ % __FUNCTION__ % module_name << std::endl;
        // TODO: throw exception
        return nullptr;
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
        paths.push_back(canonical_path);
    }

    return GetMethodFromPython(module_name, function_name, paths);
}

PyObject * CallPythonFnNullArgs(std::string module_name, std::string function_name
                                , const std::list<boost::filesystem::path> &search_paths) {
    PyObject* py_fn = GetMethodFromPython(std::move(module_name), std::move(function_name), search_paths);
    if (py_fn == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython() returned nullptr.") % __FILE__ % __FUNCTION__ << std::endl;
    }

    PyObject* py_result = PyObject_CallObject(py_fn, nullptr);

    return py_result;
}

PyObject * CallPythonFnNullArgs(PyObject *py_fn) {
    PyObject* py_result = PyObject_CallObject(py_fn, nullptr);

    return py_result;
}

PyObject * PrepareStringArgsForPython(const std::list<std::string> &args) {
    std::vector<PyObject*> py_arg_vec{};

    for (const auto& arg : args) {
        py_arg_vec.push_back(PyUnicode_FromString(arg.c_str()));
    }

    Py_ssize_t const size = static_cast<Py_ssize_t>(py_arg_vec.size());
    PyObject* py_args = PyTuple_New(size);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple") % __FILE__ % __FUNCTION__ << std::endl;
        for (auto& py_arg : py_arg_vec) {
            Py_DECREF(py_arg);
        }
        return nullptr;
    };

    for (Py_ssize_t index = 0; index < size; ++index) {
        int const err = PyTuple_SetItem(py_args, index, py_arg_vec.at(index));
        if (err < 0) {
            PyErr_Print();
            std::cerr << boost::format("%1%: %2%: Failed to add argument #%3% to PyTuple") % __FILE__ % __FUNCTION__ % index << std::endl;
            for (Py_ssize_t j = index -1; j <= 0; --j) {
                Py_DECREF(py_arg_vec.at(j));
            }
            return nullptr;
        }
    }
    return py_args;
}

PyObject * PrepareWStringArgsForPython(const std::list<std::wstring> &args) {
    std::vector<PyObject*> py_arg_vec{};

    for (const auto& arg : args) {
        py_arg_vec.push_back(PyUnicode_FromWideChar(arg.c_str(), static_cast<Py_ssize_t>(arg.length())));
    }

    Py_ssize_t const size = static_cast<Py_ssize_t>(py_arg_vec.size());
    PyObject* py_args = PyTuple_New(size);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple") % __FILE__ % __FUNCTION__ << std::endl;
        for (auto& py_arg : py_arg_vec) {
            Py_DECREF(py_arg);
        }
        return nullptr;
    };

    for (Py_ssize_t index = 0; index < size; ++index) {
        int const err = PyTuple_SetItem(py_args, index, py_arg_vec.at(index));
        if (err < 0) {
            PyErr_Print();
            std::cerr << boost::format("%1%: %2%: Failed to add argument #%3% to PyTuple") % __FILE__ % __FUNCTION__ % index << std::endl;
            for (Py_ssize_t j = index -1; j <= 0; --j) {
                Py_DECREF(py_arg_vec.at(j));
            }
            return nullptr;
        }
    }
    return py_args;
}

PyObject * CallPythonFnStringArgs(std::string module_name, std::string function_name
                                  , const std::list<boost::filesystem::path> &search_paths, const std::list<std::string> &args) {
    PyObject* py_fn = GetMethodFromPython(std::move(module_name), std::move(function_name), search_paths);
    if (py_fn == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython() returned nullptr.") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    PyObject* py_args = PrepareStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple for arguments to function call") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    PyObject* py_result = PyObject_CallObject(py_fn, py_args);

    return py_result;
}

PyObject * CallPythonFnStringArgs(PyObject *py_fn, const std::list<std::string> &args) {
    PyObject* py_args = PrepareStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: PrepareStringArgsForPython() returned nullptr") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    PyObject* py_result = PyObject_CallObject(py_fn, py_args);

    return py_result;
}

PyObject * CallPythonFnPyArgs(PyObject *py_fn, PyObject *args) {
    PyObject* py_result = PyObject_CallObject(py_fn, args);

    return py_result;
}

PyObject * CallPythonFnWStringArgs(std::string module_name, std::string function_name
                                   , const std::list<boost::filesystem::path> &search_paths, const std::list<std::wstring> &args) {
    PyObject* py_fn = GetMethodFromPython(std::move(module_name), std::move(function_name), search_paths);
    if (py_fn == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython() returned nullptr.") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    PyObject* py_args = PrepareWStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple for arguments to function call") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    PyObject* py_result = PyObject_CallObject(py_fn, py_args);

    return py_result;
}

PyObject * CallPythonFnWStringArgs(PyObject *py_fn, const std::list<std::wstring> &args) {
    PyObject* py_args = PrepareWStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple for arguments to function call") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    PyObject* py_result = PyObject_CallObject(py_fn, py_args);

    return py_result;
}

// A utility function to call a function and get a PyObject as a result
PyObject* GetClassFromPython(
    const std::string build_path,
    const std::wstring path_string,
    const std::string datadir,
    const std::string module_name,
    const std::string function_name) {

    bool py_was_initialized{};

    InitPython(py_was_initialized);

    PyObject* py_method = GetMethodFromPython(build_path, path_string, datadir, module_name, function_name);
    if (py_method == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython returned a null pointer") % __FILE__ % __FUNCTION__ << std::endl;
        if (!py_was_initialized) {
            Py_Finalize();
        }
        return nullptr;
    }

    if (!py_was_initialized) {
        Py_Finalize();
    }
    return CallPythonFnNullArgs(py_method);
}

PyObject * GetClassFromPythonPathRefs(const std::string &build_path, const std::wstring &path_string, const std::string &datadir
                                      , const std::string &module_name, const std::string &function_name) {
    return GetClassFromPython(build_path, path_string, datadir, module_name, function_name);
}

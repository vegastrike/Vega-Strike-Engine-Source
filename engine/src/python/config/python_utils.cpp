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

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <iostream>
#include <utility>

#include "python_utils.h"

#include <boost/format.hpp>

using namespace boost::python;
namespace fs = boost::filesystem;

#define Q(x) #x
#define QUOTE(x) Q(x)

#ifdef _WIN32
namespace {
/// Reusable Windows helper to pull the default Python install path from the registry
std::wstring GetPythonPathFromRegistry() {
    // Check both HKCU (current user) and HKLM (system-wide)
    HKEY root_keys[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    const wchar_t* subkey = L"SOFTWARE\\Python\\PythonCore\\3.14\\InstallPath";
    wchar_t path_buffer[MAX_PATH];
    DWORD buffer_size = sizeof(path_buffer);

    for (HKEY root : root_keys) {
        HKEY h_key;
        if (RegOpenKeyExW(root, subkey, 0, KEY_READ, &h_key) == ERROR_SUCCESS) {
            // Passing nullptr queries the default value name (the path string)
            LONG result = RegQueryValueExW(h_key, nullptr, nullptr, nullptr,
                                           reinterpret_cast<LPBYTE>(path_buffer), &buffer_size);
            RegCloseKey(h_key);
            if (result == ERROR_SUCCESS) {
                return std::wstring(path_buffer);
            }
        }
    }
    return L""; // Not found in registry
}

std::wstring GetEnvVar(const std::wstring &env_var_name) {
    wchar_t path_buffer[MAX_PATH];
    size_t required_size = 0;

    // _wgetenv_s expects the size in CHARACTER COUNT (elements), not bytes!
    errno_t err = _wgetenv_s(&required_size, path_buffer, MAX_PATH, env_var_name.c_str());

    // If the variable doesn't exist or an error occurred, return an empty string safely
    if (err != 0 || required_size == 0) {
        return L"";
    }
    return std::wstring(path_buffer);
}
}

std::wstring FindPythonPath() {
    // Step 1: Check Python_ROOT_DIR / Python3_ROOT_DIR / pythonLocation (CI consistency)
    std::wstring return_value = GetEnvVar(L"Python3_ROOT_DIR");
    if (return_value.empty()) {
        return_value = GetEnvVar(L"Python_ROOT_DIR");
    }
    if (return_value.empty()) {
        return_value = GetEnvVar(L"pythonLocation");
    }
    if (return_value.empty()) {
        return_value = GetEnvVar(L"PYTHONHOME");
    }

    // CRITICAL FIX: If any variable points to 'python.exe' or 'python_d.exe', strip it to get the directory!
    if (!return_value.empty()) {
        fs::path path_checker(return_value);
        if (path_checker.has_filename() &&
            (path_checker.filename() == "python.exe" || path_checker.filename() == "python_d.exe")) {
            return_value = path_checker.parent_path().generic_wstring();
        }
    }

    // Step 2: Fall back to PEP 514 Windows Registry Lookup
    if (return_value.empty()) {
        return_value = GetPythonPathFromRegistry();
    }

    boost::system::error_code ec;

    // Step 3: Hard Fallback to the standard user local AppData path
    if (return_value.empty()) {
        std::wstring local_app_data = GetEnvVar(L"LOCALAPPDATA");
        if (!local_app_data.empty()) {
            fs::path candidate = fs::path(local_app_data) / L"Python" / L"pythoncore-3.14-64";
            fs::path canonical_path = fs::canonical(candidate, ec);
            if (!ec) {
                return_value = canonical_path.generic_wstring();
            }
        }
    }

    // Step 4: Hard Fallback to the standard Program Files path
    if (return_value.empty()) {
        std::wstring program_files = GetEnvVar(L"ProgramFiles");
        if (program_files.empty()) {
            program_files = L"C:\\Program Files";
        }
        fs::path candidate = fs::path(program_files) / L"Python314";
        fs::path canonical_path = fs::canonical(candidate, ec);
        if (!ec) {
            return_value = canonical_path.generic_wstring();
        }
    }

    if (return_value.empty()) {
        std::cerr << "FATAL: Unable to locate Python!" << std::endl;
        exit(-1);
    }

    return return_value;
}
#endif

// This is a kludge. It runs python before
// just to get the python paths.
[[deprecated("This was always a kludge. It also uses a wide string returned from Python after Py_Finalize() has been called, which isn't safe.")]]
std::string GetPythonPath() {
    bool py_was_already_initialized = Py_IsInitialized();
    if (!py_was_already_initialized) {
        Py_Initialize();
    }

    wchar_t const* w_path_ptr = Py_GetPath();
    std::wstring w_path_w( w_path_ptr );
    std::string path( w_path_w.begin(), w_path_w.end() );

    if (!py_was_already_initialized) {
        Py_Finalize();
    }

    return path;
}

void InitPython(bool &was_already_initialized) {
    was_already_initialized = Py_IsInitialized() != 0;
    if (was_already_initialized) {
        return;
    }

    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);
    py_pre_config.coerce_c_locale = 1;

    PyStatus status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status) != 0) {
        PyErr_Print();
        Py_ExitStatusException(status);
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 0;
    config.site_import = 1;

#ifdef _WIN32
    std::wstring resolved_home = FindPythonPath();

    // Assign the determined fallback explicitly to the PyConfig structure
    status = PyConfig_SetString(&config, &config.home, resolved_home.c_str());
    if (PyStatus_Exception(status) != 0) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
    }
#endif

    if (!was_already_initialized) {
        status = Py_InitializeFromConfig(&config);
        PyConfig_Clear(&config); // Free configuration memory
        if (PyStatus_Exception(status) != 0) {
            PyErr_Print();
            Py_ExitStatusException(status);
        }
    }

UniquePyRef GetMethodFromPython(std::string module_name, std::string function_name
                                   , const std::list<fs::path> &search_paths) {
    PyObject* sys_path = PySys_GetObject("path");
    if (sys_path == nullptr) {
        std::cerr << boost::format("%1%: %2%: PySys_GetObject(\"path\") returned nullptr")
                % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    for (const auto& search_path : search_paths) {
        fs::path const canonical_path = fs::canonical(search_path);
        std::string const canonical_path_str = canonical_path.generic_string();
        std::wstring const canonical_path_wstr = canonical_path.generic_wstring();
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

    UniquePyRef module{PyImport_ImportModule(module_name.c_str())};
    if (module == nullptr) {
        PyErr_Print();
        std::cerr << boost::format("%1%: %2%: PyImport_ImportModule() returned nullptr for module %3%")
                % __FILE__ % __FUNCTION__ % module_name << std::endl;
        return nullptr;
    }

    UniquePyRef func(PyObject_GetAttrString(module.get(), function_name.c_str()));
    if (func == nullptr) {
        std::cerr << boost::format("%1%: %2%: PyObject_GetAttrString returned nullptr for function %3%")
            % __FILE__ % __FUNCTION__ % function_name << std::endl;
        PyErr_Print();
        return nullptr;
    }
    if (!PyCallable_Check(func.get())) {
        std::cerr << boost::format("%1%: %2%: PyObject_GetAttrString did not return a callable object for function %3%")
            % __FILE__ % __FUNCTION__ % function_name << std::endl;
        PyErr_Print();
        return nullptr;
    }

    return func;
}

UniquePyRef GetMethodFromPython(std::string module_name,
                                   std::string function_name,
                                   const std::list<std::string> &search_paths) {
    std::list<fs::path> paths;
    for (const auto& path : search_paths) {
        fs::path const canonical_path = fs::canonical(path);
        paths.push_back(canonical_path);
    }

    return GetMethodFromPython(std::move(module_name), std::move(function_name), paths);
}

UniquePyRef GetMethodFromPython(std::string module_name,
                                   std::string function_name,
                                   const std::list<std::wstring> &search_paths) {
    std::list<fs::path> paths;
    for (const auto& path : search_paths) {
        fs::path const canonical_path = fs::canonical(path);
        paths.push_back(canonical_path);
    }

    return GetMethodFromPython(std::move(module_name), std::move(function_name), paths);
}

UniquePyRef GetMethodFromPython(const std::string &build_path, const std::wstring &path_string
                                   , const std::string &datadir, const std::string &module_name
                                   , const std::string &function_name) {
    fs::path const full_path = fs::path(path_string);
    fs::current_path(full_path);

    if(!fs::exists(module_name + ".py")) {
        std::cerr << boost::format("%1%: %2%: Can't find module %3%.") % __FILE__ % __FUNCTION__ % module_name << std::endl;
        // TODO: throw exception
        return nullptr;
    }

    const std::wstring path_string_w = fs::canonical(path_string).generic_wstring();
    const std::string python_sitelib_path = QUOTE(Python_SITELIB);
    const std::wstring python_sitelib_path_wstring = fs::canonical(python_sitelib_path).generic_wstring();
    const std::wstring build_path_w = fs::canonical(build_path).generic_wstring();
    const std::wstring data_path_w = fs::canonical(datadir).generic_wstring();

    std::list<std::wstring> const path_wstrs{path_string_w, python_sitelib_path_wstring, build_path_w, data_path_w};
    std::list<fs::path> paths;
    for (const auto& path : path_wstrs) {
        fs::path const canonical_path = fs::canonical(path);
        paths.push_back(canonical_path);
    }

    return GetMethodFromPython(module_name, function_name, paths);
}

UniquePyRef CallPythonFnNullArgs(std::string module_name, std::string function_name
                                    , const std::list<fs::path> &search_paths) {
    UniquePyRef py_fn = GetMethodFromPython(std::move(module_name), std::move(function_name), search_paths);
    if (py_fn == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython() returned nullptr.") % __FILE__ % __FUNCTION__ << std::endl;
    }

    return UniquePyRef(PyObject_CallObject(py_fn.get(), nullptr));
}

UniquePyRef CallPythonFnNullArgs(PyObject *py_fn) {
    return UniquePyRef(PyObject_CallObject(py_fn, nullptr));
}

UniquePyRef CallPythonFnNullArgs(UniquePyRef py_fn) {
    return CallPythonFnNullArgs(py_fn.get());
}

UniquePyRef PrepareStringArgsForPython(const std::list<std::string> &args) {
    Py_ssize_t const size = static_cast<Py_ssize_t>(args.size());
    UniquePyRef py_args{PyTuple_New(size)};
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    Py_ssize_t index = 0;
    for (const auto& arg : args) {
        PyObject* py_str = PyUnicode_FromString(arg.c_str());
        if (py_str == nullptr) {
            std::cerr << boost::format("%1%: %2%: Failed to convert string argument #%3%") % __FILE__ % __FUNCTION__ % index << std::endl;
            return nullptr; // py_args destructor cleans up any items already set
        }

        // PyTuple_SetItem steals the reference to py_str on success
        int const err = PyTuple_SetItem(py_args.get(), index, py_str);
        if (err < 0) {
            Py_DECREF(py_str); // Clean up manually if the steal failed
            PyErr_Print();
            std::cerr << boost::format("%1%: %2%: Failed to add argument #%3% to PyTuple") % __FILE__ % __FUNCTION__ % index << std::endl;
            return nullptr;
        }
        ++index;
    }
    return py_args;
}

UniquePyRef PrepareWStringArgsForPython(const std::list<std::wstring> &args) {
    Py_ssize_t const size = static_cast<Py_ssize_t>(args.size());
    UniquePyRef py_args{PyTuple_New(size)};
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    Py_ssize_t index = 0;
    for (const auto& arg : args) {
        PyObject* py_wstr = PyUnicode_FromWideChar(arg.c_str(), static_cast<Py_ssize_t>(arg.length()));
        if (py_wstr == nullptr) {
            std::cerr << boost::format("%1%: %2%: Failed to convert wstring argument #%3%") % __FILE__ % __FUNCTION__ % index << std::endl;
            return nullptr;
        }

        int const err = PyTuple_SetItem(py_args.get(), index, py_wstr);
        if (err < 0) {
            Py_DECREF(py_wstr); // Clean up manually if the steal failed
            PyErr_Print();
            std::cerr << boost::format("%1%: %2%: Failed to add argument #%3% to PyTuple") % __FILE__ % __FUNCTION__ % index << std::endl;
            return nullptr;
        }
        ++index;
    }
    return py_args;
}

// FIXED: Changed return signature to UniquePyRef to plug caller-side leaks
UniquePyRef CallPythonFnStringArgs(std::string module_name, std::string function_name
                                      , const std::list<fs::path> &search_paths, const std::list<std::string> &args) {
    UniquePyRef py_fn = GetMethodFromPython(std::move(module_name), std::move(function_name), search_paths);
    if (py_fn == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython() returned nullptr.") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    UniquePyRef py_args = PrepareStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple for arguments to function call") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    // Safely wrap the result pointer right out of invocation
    return UniquePyRef(PyObject_CallObject(py_fn.get(), py_args.get()));
}

UniquePyRef CallPythonFnStringArgs(PyObject *py_fn, const std::list<std::string> &args) {
    UniquePyRef py_args = PrepareStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: PrepareStringArgsForPython() returned nullptr") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    return UniquePyRef(PyObject_CallObject(py_fn, py_args.get()));
}

UniquePyRef CallPythonFnStringArgs(UniquePyRef py_fn, const std::list<std::string> &args) {
    return CallPythonFnStringArgs(py_fn.get(), args);
}

UniquePyRef CallPythonFnPyArgs(PyObject *py_fn, PyObject *args) {
    return UniquePyRef(PyObject_CallObject(py_fn, args));
}

UniquePyRef CallPythonFnPyArgs(UniquePyRef py_fn, PyObject *args) {
    return CallPythonFnPyArgs(py_fn.get(), args);
}

UniquePyRef CallPythonFnPyArgs(PyObject *py_fn, UniquePyRef args) {
    return CallPythonFnPyArgs(py_fn, args.get());
}

UniquePyRef CallPythonFnPyArgs(UniquePyRef py_fn, UniquePyRef args) {
    return CallPythonFnPyArgs(py_fn.get(), args.get());
}

UniquePyRef CallPythonFnWStringArgs(std::string module_name, std::string function_name
                                       , const std::list<fs::path> &search_paths, const std::list<std::wstring> &args) {
    UniquePyRef py_fn = GetMethodFromPython(std::move(module_name), std::move(function_name), search_paths);
    if (py_fn == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython() returned nullptr.") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    UniquePyRef py_args = PrepareWStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple for arguments to function call") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    return UniquePyRef(PyObject_CallObject(py_fn.get(), py_args.get()));
}

UniquePyRef CallPythonFnWStringArgs(PyObject *py_fn, const std::list<std::wstring> &args) {
    UniquePyRef py_args = PrepareWStringArgsForPython(args);
    if (py_args == nullptr) {
        std::cerr << boost::format("%1%: %2%: Failed to create PyTuple for arguments to function call") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }

    return UniquePyRef(PyObject_CallObject(py_fn, py_args.get()));
}

UniquePyRef CallPythonFnWStringArgs(UniquePyRef py_fn, const std::list<std::wstring> &args) {
    return CallPythonFnWStringArgs(py_fn.get(), args);
}

UniquePyRef GetClassFromPython(
        const std::string build_path,
        const std::wstring path_string,
        const std::string datadir,
        const std::string module_name,
        const std::string function_name) {
    UniquePyRef py_method = GetMethodFromPython(build_path, path_string, datadir, module_name, function_name);
    if (py_method == nullptr) {
        std::cerr << boost::format("%1%: %2%: GetMethodFromPython returned a null pointer") % __FILE__ % __FUNCTION__ << std::endl;
        return nullptr;
    }
    // FIXED: Safely return the UniquePyRef instead of using .get()
    return CallPythonFnNullArgs(std::move(py_method));
}

UniquePyRef GetClassFromPythonPathRefs(const std::string &build_path, const std::wstring &path_string
                                          , const std::string &datadir
                                          , const std::string &module_name, const std::string &function_name) {
    return GetClassFromPython(build_path, path_string, datadir, module_name, function_name);
}

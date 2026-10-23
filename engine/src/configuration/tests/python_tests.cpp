/*
 * python_tests.cpp
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
#include <boost/python.hpp>
#include <Python.h>
#include <boost/version.hpp>
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <iostream>

#include "src/python/config/python_utils.h"

using namespace boost::python;


struct World
{
    World(std::string msg) : msg(msg) {}
    void set(std::string msg) { this->msg = msg; }
    std::string greet() { return msg; }
    std::string msg;
};

BOOST_PYTHON_MODULE(python_sanity)
{
    class_<World>("World", init<std::string>())
        .def("greet", &World::greet)
        .def("set", &World::set)
    ;
}

const std::string hello_world = "Hello World\n";


TEST(Python, Sanity) {
    World world(hello_world);

    EXPECT_EQ(world.greet(), hello_world);
}

TEST(Python, Call_Function) {
    // Structural, platform-agnostic path building
    boost::filesystem::path current_dir = boost::filesystem::current_path();
    std::cout << "Original structural path: " << current_dir << std::endl;

    // Use structural appending '/=' instead of a flat string concat
    boost::filesystem::path target_path = current_dir / ".." / "test_assets";

    // Normalize path to clean out the relative '..' steps into an absolute path
    boost::filesystem::path test_assets_path = boost::filesystem::canonical(target_path);
    std::cout << "Normalized test assets directory: " << test_assets_path << std::endl;

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path);
    bool exists = boost::filesystem::exists("python_tests.py");
    std::cout << "python_tests.py exists locally: " << exists << std::endl;

    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);
    py_pre_config.coerce_c_locale = 1;

    PyStatus status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 0;

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config); // Free configuration memory
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    // 3. Convert path cleanly depending on platform context type
#if defined(_WIN32)
    std::wstring test_assets_str = test_assets_path.wstring();
#else
    // On Linux/macOS, convert native std::string path into a safe wstring for the C-API
    std::string native_path = test_assets_path.string();
    std::wstring test_assets_str(native_path.begin(), native_path.end());
#endif

    // Grab the live sys.path list object
    PyObject* sys_path = PySys_GetObject("path");
    ASSERT_NE(sys_path, nullptr) << "Failed to access Python sys.path!";

    // Create a Python string element matching our path type
    PyObject* path_element = PyUnicode_FromWideChar(test_assets_str.c_str(), test_assets_str.length());
    ASSERT_NE(path_element, nullptr) << "Failed to allocate Unicode path string.";

    // Safely append directly into Python's runtime search paths array
    int append_status = PyList_Append(sys_path, path_element);
    Py_DECREF(path_element); // Clean up temporary reference allocation
    ASSERT_EQ(append_status, 0) << "Failed to append test directory to sys.path.";

    // Test execution (Fail early and loudly if things go missing)
    PyObject* module = PyImport_ImportModule("python_tests");
    ASSERT_NE(module, nullptr) << "Failed to import python_tests module! Check module paths.";

    PyObject* function = PyObject_GetAttrString(module, "append");
    ASSERT_NE(function, nullptr) << "Failed to load 'append' attribute from Python script.";

    PyObject* args = PyTuple_Pack(2,
                                  PyUnicode_FromString("Hello "),
                                  PyUnicode_FromString("World\n"));
    ASSERT_NE(args, nullptr) << "Failed to create parameter tuple pack.";

    PyObject* pyResult = PyObject_CallObject(function, args);
    ASSERT_NE(pyResult, nullptr) << "Python function call returned a null pointer evaluation result.";

    std::string result = PyUnicode_AsUTF8(pyResult);
    Py_Finalize();

    EXPECT_EQ(result, hello_world);
}

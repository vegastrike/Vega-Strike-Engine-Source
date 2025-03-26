/*
 * python_tests.cpp
 *
 * Copyright (c) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <boost/python.hpp>
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
    boost::filesystem::path full_path(boost::filesystem::current_path());
    std::cout << full_path << std::endl;

    bool exists = boost::filesystem::exists("../test_assets/python_tests.py");
    std::cout << "python_tests.py exists " << exists << std::endl;

    boost::filesystem::current_path(full_path.concat("/../test_assets"));
    full_path = boost::filesystem::current_path();
    std::cout << full_path << std::endl;

    exists = boost::filesystem::exists("python_tests.py");
    std::cout << "python_tests.py exists in current folder " << exists << std::endl;

    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);

    PyStatus status;

    status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    const std::string python_path = GetPythonPath();
    const std::wstring python_path_w(python_path.begin(), python_path.end());

    PyWideStringList python_path_py_wide_string_list{};
    status = PyWideStringList_Append(&python_path_py_wide_string_list, python_path_w.c_str());
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    config.module_search_paths = python_path_py_wide_string_list;
    config.isolated = 0;

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
    }

    PyObject* module = PyImport_ImportModule("python_tests");
    std::cout << "PyImport_ImportModule did not crash\n" << std::flush;
    if(!module) {
        std::cout << "PyImport_ImportModule is null\n" << std::flush;
        PyErr_Print();
        Py_Finalize();
        return;
    } else {
        std::cout << "PyImport_ImportModule is not null\n" << std::flush;
    }

    PyObject* function = PyObject_GetAttrString(module,"append");
    std::cout << "PyObject_GetAttrString did not crash\n" << std::flush;
    if(function == nullptr) {
        std::cout << "PyObject_GetAttrString is null\n" << std::flush;
        //PyErr_Print();
        Py_Finalize();
        return;
    } else {
        std::cout << "PyObject_GetAttrString is not null\n" << std::flush;
    }

    PyObject* args = PyTuple_Pack(2,
                                  PyUnicode_FromString("Hello "),
                                  PyUnicode_FromString("World\n"));
    std::cout << "PyTuple_Pack did not crash\n" << std::flush;
    if(args == nullptr) {
        std::cout << "PyTuple_Pack is null\n" << std::flush;
        PyErr_Print();
        Py_Finalize();
        return;
    } else {
        std::cout << "PyTuple_Pack is not null\n" << std::flush;
    }

    PyObject* pyResult = PyObject_CallObject(function, args);
    std::cout << "PyObject_CallObject\n";
    std::string result = PyUnicode_AsUTF8(pyResult);
    std::cout << result << std::endl << std::flush;
    Py_Finalize();

    EXPECT_EQ(result, hello_world);

    // Uncomment to see prints
    //EXPECT_FALSE(true);
}

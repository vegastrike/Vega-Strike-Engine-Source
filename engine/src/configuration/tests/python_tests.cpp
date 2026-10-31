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
#include <memory>
#include <vector>

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

namespace {
std::shared_ptr<boost::filesystem::path> FindTestPath(const boost::filesystem::path &starting_path
                                                      , std::string const &relative_path) {
    boost::filesystem::path const canonical_starting_path = boost::filesystem::canonical(starting_path).
            lexically_normal();
    boost::filesystem::path const root_path = canonical_starting_path.root_path();
    boost::filesystem::path base_path_to_try{canonical_starting_path};
    for (; !boost::filesystem::is_directory(base_path_to_try / relative_path) && base_path_to_try != root_path;
           base_path_to_try = boost::filesystem::canonical(base_path_to_try.parent_path()).lexically_normal()) {
           }
    if (boost::filesystem::is_directory(base_path_to_try / relative_path)) {
        std::shared_ptr<boost::filesystem::path> result = std::make_shared<boost::filesystem::path>(
                boost::filesystem::canonical(boost::filesystem::path(base_path_to_try / relative_path)).
                lexically_normal());
        std::cout << "Found path: '" << relative_path << "' at '" << result->string() << "'\n";
        return result;
    } else {
        std::cout << "Did not find path: '" << relative_path << "' starting from '" << starting_path << "'\n";
        return nullptr;
    }
}

std::shared_ptr<std::vector<std::shared_ptr<boost::filesystem::path> > > TestPathsHelper() {
    // Structural, platform-agnostic path building
    boost::filesystem::path const current_dir = boost::filesystem::current_path();
    std::cout << "Original structural path: " << current_dir << '\n';
    std::shared_ptr<boost::filesystem::path> current_dir_canonical_ptr = std::make_shared<boost::filesystem::path>(
            boost::filesystem::canonical(current_dir).lexically_normal());
    std::cout << "Canonicalized original path: " << *current_dir_canonical_ptr << '\n';

    boost::filesystem::path const up_one_level = current_dir_canonical_ptr->parent_path();
    std::shared_ptr<boost::filesystem::path> up_one_level_canonical_ptr = std::make_shared<boost::filesystem::path>(
            boost::filesystem::canonical(up_one_level).lexically_normal());
    std::cout << "Up one level: " << *up_one_level_canonical_ptr << '\n';

    std::shared_ptr<boost::filesystem::path> test_assets_path_canonical_ptr = FindTestPath(
            *current_dir_canonical_ptr, "test_assets");
    if (test_assets_path_canonical_ptr == nullptr) {
        return nullptr;
    }
    std::cout << "Normalized test assets directory: " << *test_assets_path_canonical_ptr << '\n';

    boost::filesystem::path const build_path = *up_one_level_canonical_ptr;
    std::shared_ptr<boost::filesystem::path> build_path_canonical_ptr = std::make_shared<
        boost::filesystem::path>(build_path);
    std::cout << "Build path: " << *build_path_canonical_ptr << '\n';

    // Shift context location explicitly
    boost::filesystem::current_path(*test_assets_path_canonical_ptr);
    bool const exists = boost::filesystem::exists("python_tests.py");
    std::cout << "python_tests.py exists locally: " << exists << '\n';

    std::shared_ptr<boost::filesystem::path> data_path_canonical_ptr = FindTestPath(
             *current_dir_canonical_ptr, "Assets-Production");
    if (data_path_canonical_ptr == nullptr) {
        data_path_canonical_ptr = test_assets_path_canonical_ptr;
    }
    std::cout << "data_path: " << *data_path_canonical_ptr << '\n';
    std::shared_ptr<std::vector<std::shared_ptr<boost::filesystem::path> > > search_paths_vec_ptr = std::make_shared<
        std::vector<std::shared_ptr<boost::filesystem::path> > >();
    search_paths_vec_ptr->emplace_back(build_path_canonical_ptr);
    search_paths_vec_ptr->emplace_back(test_assets_path_canonical_ptr);
    search_paths_vec_ptr->emplace_back(data_path_canonical_ptr);
    return search_paths_vec_ptr;
}
}

TEST(Python, FindTestPath_failure_case) {
    boost::filesystem::path const current_dir = boost::filesystem::current_path();
    std::shared_ptr<boost::filesystem::path> current_dir_canonical_ptr = std::make_shared<boost::filesystem::path>(
            boost::filesystem::canonical(current_dir).lexically_normal());
    std::shared_ptr<boost::filesystem::path> should_be_null{};
    ASSERT_NO_THROW(should_be_null = FindTestPath(*current_dir_canonical_ptr, "nonexistent"));
    ASSERT_EQ(should_be_null, nullptr) << "FindTestPath should return nullptr\n";
}

TEST(Python, FindTestPath_success_case) {
    boost::filesystem::path const current_dir = boost::filesystem::current_path();
    std::shared_ptr<boost::filesystem::path> current_dir_canonical_ptr = std::make_shared<boost::filesystem::path>(
            boost::filesystem::canonical(current_dir).lexically_normal());
    std::shared_ptr<boost::filesystem::path> should_not_be_null{};
    ASSERT_NO_THROW(should_not_be_null = FindTestPath(*current_dir_canonical_ptr, "test_assets"));
    ASSERT_NE(should_not_be_null, nullptr) << "FindTestPath should not return nullptr\n";
}

TEST(Python, Call_Function) {
    std::shared_ptr<std::vector<std::shared_ptr<boost::filesystem::path> > > search_paths_vec_ptr = TestPathsHelper();
    std::shared_ptr<boost::filesystem::path> test_assets_path_ptr = search_paths_vec_ptr->at(1);
    bool const exists = boost::filesystem::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);
    py_pre_config.coerce_c_locale = 1;

    PyStatus status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status) != 0) {
        Py_ExitStatusException(status);
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 0;

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config); // Free configuration memory
    if (PyStatus_Exception(status) != 0) {
        Py_ExitStatusException(status);
    }

    // Convert path cleanly
    std::wstring const test_assets_str = test_assets_path_ptr->generic_wstring();

    // Grab the live sys.path list object
    PyObject *sys_path = PySys_GetObject("path");
    ASSERT_NE(sys_path, nullptr) << "Failed to access Python sys.path!";

    // Create a Python string element matching our path type
    PyObject *path_element = PyUnicode_FromWideChar(test_assets_str.c_str(), test_assets_str.length());
    ASSERT_NE(path_element, nullptr) << "Failed to allocate Unicode path string.";

    // Safely append directly into Python's runtime search paths array
    int const append_status = PyList_Append(sys_path, path_element);
    Py_DECREF(path_element); // Clean up temporary reference allocation
    ASSERT_EQ(append_status, 0) << "Failed to append test directory to sys.path.";

    // Test execution (Fail early and loudly if things go missing)
    PyObject *module = PyImport_ImportModule("python_tests");
    ASSERT_NE(module, nullptr) << "Failed to import python_tests module! Check module paths.";

    PyObject *function = PyObject_GetAttrString(module, "append");
    ASSERT_NE(function, nullptr) << "Failed to load 'append' attribute from Python script.";

    PyObject *args = PyTuple_Pack(2,
                                  PyUnicode_FromString("Hello "),
                                  PyUnicode_FromString("World\n"));
    ASSERT_NE(args, nullptr) << "Failed to create parameter tuple pack.";

    PyObject *py_result = PyObject_CallObject(function, args);
    ASSERT_NE(py_result, nullptr) << "Python function call returned a null pointer evaluation result.";

    std::string const result = PyUnicode_AsUTF8(py_result);

    Py_Finalize();

    EXPECT_EQ(result, hello_world);

    // Uncomment to see prints
    //EXPECT_FALSE(true);
}

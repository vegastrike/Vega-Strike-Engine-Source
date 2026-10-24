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

const std::string kHelloWorldPascalCase = "Hello World\n";
const std::string kHelloWorldLowercase = "hello world";

class PythonTests : public ::testing::Test {
protected:
    static bool python_was_initialized_;
    static void SetUpTestSuite() {
        std::cout << "Initializing Python..." << std::endl;
        InitPython(python_was_initialized_);
    }

    static void TearDownTestSuite() {
        if (!python_was_initialized_) {
            std::cout << "Tearing down Python..." << std::endl;
            Py_Finalize();
        }
    }
};

bool PythonTests::python_was_initialized_ = false;

TEST_F(PythonTests, Sanity) {
    World world(kHelloWorldPascalCase);

    EXPECT_EQ(world.greet(), kHelloWorldPascalCase);
}

TEST_F(PythonTests, Call_Function) {
    // Structural, platform-agnostic path building
    boost::filesystem::path const current_dir = boost::filesystem::current_path();
    std::cout << "Original structural path: " << current_dir << '\n';

    // Use structural appending '/=' instead of a flat string concat
    boost::filesystem::path const target_path = current_dir / ".." / "test_assets";

    // Normalize path to clean out the relative '..' steps into an absolute path
    boost::filesystem::path const test_assets_path = boost::filesystem::canonical(target_path);
    std::cout << "Normalized test assets directory: " << test_assets_path << '\n';

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path);
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
    std::wstring const test_assets_str = test_assets_path.lexically_normal().generic_wstring();

    // Grab the live sys.path list object
    PyObject* sys_path = PySys_GetObject("path");
    ASSERT_NE(sys_path, nullptr) << "Failed to access Python sys.path!";

    // Create a Python string element matching our path type
    PyObject* path_element = PyUnicode_FromWideChar(test_assets_str.c_str(), test_assets_str.length());
    ASSERT_NE(path_element, nullptr) << "Failed to allocate Unicode path string.";

    // Safely append directly into Python's runtime search paths array
    int const append_status = PyList_Append(sys_path, path_element);
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

    PyObject* py_result = PyObject_CallObject(function, args);
    ASSERT_NE(py_result, nullptr) << "Python function call returned a null pointer evaluation result.";

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, InitPython) {
    bool python_was_initialized{};
    // This function will be called even though Python was already init'ed at the test suite level. But that's OK,
    // because InitPython checks to see if Python was already init'ed before initializing it again.
    InitPython(python_was_initialized);
}

namespace {
std::vector<boost::filesystem::path> TestPathsHelper() {
    // Structural, platform-agnostic path building
    boost::filesystem::path const current_dir = boost::filesystem::current_path();
    std::cout << "Original structural path: " << current_dir << '\n';

    boost::filesystem::path const up_one_level = current_dir / "..";
    boost::filesystem::path const up_one_level_canonical = boost::filesystem::canonical(up_one_level).lexically_normal();
    std::cout << "Up one level: " << up_one_level_canonical << '\n';

    boost::filesystem::path const test_assets_path = up_one_level_canonical / "test_assets";
    boost::filesystem::path const test_assets_path_canonical = boost::filesystem::canonical(test_assets_path).lexically_normal();
    std::cout << "Normalized test assets directory: " << test_assets_path_canonical << '\n';

    boost::filesystem::path const build_path = up_one_level_canonical / "..";
    boost::filesystem::path const build_path_canonical = boost::filesystem::canonical(build_path).lexically_normal();

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path_canonical);
    bool const exists = boost::filesystem::exists("python_tests.py");
    std::cout << "python_tests.py exists locally: " << exists << '\n';

    boost::filesystem::path const data_path = up_one_level_canonical / ".." / ".." / ".." / "Assets-Production";
    boost::filesystem::path const data_path_canonical = boost::filesystem::canonical(data_path).lexically_normal();
    std::cout << "data_path: " << data_path_canonical << '\n';

    std::vector<boost::filesystem::path> const search_paths{build_path_canonical, test_assets_path_canonical, data_path_canonical};
    return search_paths;
}
}

TEST_F(PythonTests, GetClassFromPython) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    const boost::filesystem::path& build_path = search_paths.at(0);
    const boost::filesystem::path& test_assets_path = search_paths.at(1);
    const boost::filesystem::path& data_path = search_paths.at(2);

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path);
    bool const exists = boost::filesystem::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyObject* py_result = GetClassFromPython(build_path.generic_string(), test_assets_path.generic_wstring(), data_path.generic_string(), "python_tests", "append_no_args");
    ASSERT_NE(py_result, nullptr) << "Python result is null";

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetClassFromPythonPathRefs) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    const boost::filesystem::path& build_path = search_paths.at(0);
    const boost::filesystem::path& test_assets_path = search_paths.at(1);
    const boost::filesystem::path& data_path = search_paths.at(2);

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path);
    bool const exists = boost::filesystem::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyObject* py_result = GetClassFromPython(build_path.generic_string(), test_assets_path.generic_wstring(), data_path.generic_string(), "python_tests", "append_no_args");
    ASSERT_NE(py_result, nullptr) << "Python result is null";

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_list_of_boost_filesystem_paths) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    const boost::filesystem::path& build_path = search_paths.at(0);
    const boost::filesystem::path& test_assets_path = search_paths.at(1);
    const boost::filesystem::path& data_path = search_paths.at(2);

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path);
    bool const exists = boost::filesystem::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    std::list<boost::filesystem::path> const search_path_list{test_assets_path, build_path, data_path};

    PyObject* py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject* py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_list_of_strings) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<std::string> search_path_strings{};
    for (const auto& search_path : search_paths) {
        search_path_strings.emplace_back(search_path.generic_string());
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_strings);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject* py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_list_of_wstrings) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<std::wstring> search_path_strings{};
    for (const auto& search_path : search_paths) {
        search_path_strings.emplace_back(search_path.generic_wstring());
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_strings);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject* py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_individual_paths) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    const boost::filesystem::path& build_path = search_paths.at(0);
    const boost::filesystem::path& test_assets_path = search_paths.at(1);
    const boost::filesystem::path& data_path = search_paths.at(2);

    // Shift context location explicitly
    boost::filesystem::current_path(test_assets_path);
    bool const exists = boost::filesystem::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyObject* py_method = GetMethodFromPython(build_path.generic_string(), test_assets_path.generic_wstring(), data_path.generic_string(), "python_tests", "append_no_args");
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject* py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, PrepareStringArgsForPython) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    const std::list<std::string> arg_string_list{"Hello", " World\n"};
    PyObject* py_args = PrepareStringArgsForPython(arg_string_list);
    ASSERT_NE(py_args, nullptr) << "PrepareStringArgsForPython returned a null pointer" << '\n';

    PyObject* py_result = CallPythonFnPyArgs(py_method, py_args);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnPyArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, PrepareWStringArgsForPython) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject* py_args = PrepareWStringArgsForPython(arg_wstring_list);
    ASSERT_NE(py_args, nullptr) << "PrepareWStringArgsForPython returned a null pointer" << '\n';

    PyObject* py_result = CallPythonFnPyArgs(py_method, py_args);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnPyArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnNullArgs_with_module_name_fn_name_and_search_paths) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_result = CallPythonFnNullArgs("python_tests", "append_no_args", search_path_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, CallPythonFnNullArgs_with_fn_pointer) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    PyObject* py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, CallPythonFnStringArgs_with_module_name_fn_name_and_search_paths) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    const std::list<std::string> arg_string_list{"Hello", " World\n"};
    PyObject* py_result = CallPythonFnStringArgs("python_tests", "append", search_path_list, arg_string_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnStringArgs_with_fn_pointer) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    const std::list<std::string> arg_string_list{"Hello", " World\n"};
    PyObject* py_result = CallPythonFnStringArgs(py_method, arg_string_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnWstringArgs_with_module_name_fn_name_and_search_paths) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject* py_result = CallPythonFnWStringArgs("python_tests", "append", search_path_list, arg_wstring_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnWStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnWstringArgs_with_fn_pointer) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject* py_result = CallPythonFnWStringArgs(py_method, arg_wstring_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnWStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnPyArgs) {
    std::vector<boost::filesystem::path> const search_paths = TestPathsHelper();
    std::list<boost::filesystem::path> search_path_list{};
    for (const auto& search_path : search_paths) {
        search_path_list.emplace_back(search_path);
    }

    PyObject* py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject* py_args = PrepareWStringArgsForPython(arg_wstring_list);
    ASSERT_NE(py_args, nullptr) << "PrepareWStringArgsForPython returned a null pointer" << '\n';

    PyObject* py_result = CallPythonFnPyArgs(py_method, py_args);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnPyArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

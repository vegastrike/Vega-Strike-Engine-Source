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
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "src/python/config/python_utils.h"

using namespace boost::python;
namespace fs = boost::filesystem;

struct World {
    World(std::string msg) : msg(msg) {
    }

    void set(std::string msg) {
        this->msg = msg;
    }

    std::string greet() {
        return msg;
    }

    std::string msg;
};

BOOST_PYTHON_MODULE(python_sanity) {
    class_<World>("World", init<std::string>())
            .def("greet", &World::greet)
            .def("set", &World::set);
}

const std::string kHelloWorldPascalCase = "Hello World\n";
const std::string kHelloWorldLowercase = "hello world";

namespace {
std::shared_ptr<fs::path> FindTestPath(const fs::path &starting_path, std::string const &relative_path) {
    boost::system::error_code ec;
    fs::path dir = fs::canonical(starting_path, ec);

    // Fallback if starting_path doesn't exist
    if (ec) {
        dir = fs::absolute(starting_path);
    }

    // Keep moving up until the path becomes empty (past the root)
    while (!dir.empty()) {
        fs::path candidate = dir / relative_path;

        if (fs::is_directory(candidate)) {
            fs::path canonical_candidate = fs::canonical(candidate, ec);
            if (!ec) {
                auto result = std::make_shared<fs::path>(std::move(canonical_candidate));
                std::cout << "Found path: '" << relative_path << "' at '" << result->string() << "'\n";
                return result;
            }
        }

        // Break early if we just checked the root directory, preventing it from turning into ""
        if (dir == dir.root_path()) {
            break;
        }

        dir = dir.parent_path();
    }

    std::cout << "Did not find path: '" << relative_path << "' starting from '" << starting_path << "'\n";
    return nullptr;
}
}

std::shared_ptr<std::vector<std::shared_ptr<fs::path>>> GetTestPaths() {
    // Global cache variables with internal linkage
    static std::shared_ptr<std::vector<std::shared_ptr<fs::path>>> cached_paths = nullptr;
    static std::once_flag cache_flag;

    // 1. Compute the paths exactly ONCE across the entire application lifetime
    std::call_once(cache_flag, []() {
        boost::system::error_code ec;
        const fs::path start = fs::canonical(fs::current_path(), ec);
        // Fail early if current_path is invalid
        if (ec) {
            return;
        }

        const auto test_assets = FindTestPath(start, "test_assets");
        if (!test_assets) return;

        auto data = FindTestPath(start, "Assets-Production");
        if (!data) {
            data = test_assets;
        }

        const auto build = std::make_shared<fs::path>(start.parent_path());

        // Construct the global vector cache
        cached_paths = std::make_shared<std::vector<std::shared_ptr<fs::path>>>(
            std::vector<std::shared_ptr<fs::path>>{build, test_assets, data}
        );
    });

    // 2. Handle the case where path resolution failed during initialization
    if (!cached_paths) {
        return nullptr;
    }

    return cached_paths;
}

class TestEnvironment : public ::testing::Environment {
public:
    ~TestEnvironment() override {}

    // Override SetUp to run before any test suites execute
    void SetUp() override {
        const auto paths = GetTestPaths();
        if (paths && paths->size() >= 2) {
            const fs::path& test_assets = *(paths->at(1)); // 'test_assets'

            boost::system::error_code ec;
            fs::current_path(test_assets, ec);
            if (ec) {
                std::cerr << "CRITICAL: Failed to set global working directory to: "
                          << test_assets << "\n";
            } else {
                std::cout << "Global working directory set to: " << test_assets << "\n";
            }
        }
    }

    // Override TearDown if you need to clean up global state (optional)
    void TearDown() override {}
};

// This static variable registers the environment automatically before main() runs!
static ::testing::Environment* const g_gtest_env = ::testing::AddGlobalTestEnvironment(new TestEnvironment);

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

TEST_F(PythonTests, FindTestPath_failure_case) {
    fs::path const current_dir = fs::canonical(fs::current_path());
    std::shared_ptr<fs::path> should_be_null{};
    ASSERT_NO_THROW(should_be_null = FindTestPath(current_dir, "nonexistent"));
    ASSERT_EQ(should_be_null, nullptr) << "FindTestPath should return nullptr\n";
}

TEST_F(PythonTests, FindTestPath_success_case) {
    fs::path const current_dir = fs::canonical(fs::current_path());
    std::shared_ptr<fs::path> should_not_be_null{};
    ASSERT_NO_THROW(should_not_be_null = FindTestPath(current_dir, "test_assets"));
    ASSERT_NE(should_not_be_null, nullptr) << "FindTestPath should not return nullptr\n";
}

TEST_F(PythonTests, Call_Function) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    std::shared_ptr<fs::path> test_assets_path_ptr = search_paths_vec_ptr->at(1);
    bool const exists = fs::exists("python_tests.py");
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

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, InitPython) {
    bool python_was_initialized{};
    // This function will be called even though Python was already init'ed at the test suite level. But that's OK,
    // because InitPython checks to see if Python was already init'ed before initializing it again.
    InitPython(python_was_initialized);
}

TEST_F(PythonTests, GetClassFromPython) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::shared_ptr<fs::path> build_path_ptr = search_paths_vec_ptr->at(0);
    ASSERT_NE(build_path_ptr, nullptr) << "Test build path should not be null!";
    std::shared_ptr<fs::path> test_assets_path_ptr = search_paths_vec_ptr->at(1);
    ASSERT_NE(test_assets_path_ptr, nullptr) << "test_assets path should not be null!";
    std::shared_ptr<fs::path> data_path_ptr = search_paths_vec_ptr->at(2);
    ASSERT_NE(data_path_ptr, nullptr) << "data path should not be null!";

    bool const exists = fs::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyObject *py_result = GetClassFromPython(build_path_ptr->generic_string(), test_assets_path_ptr->generic_wstring()
                                             , data_path_ptr->generic_string(), "python_tests", "append_no_args");
    ASSERT_NE(py_result, nullptr) << "Python result is null";

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetClassFromPythonPathRefs) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::shared_ptr<fs::path> build_path_ptr = search_paths_vec_ptr->at(0);
    ASSERT_NE(build_path_ptr, nullptr) << "Test build path should not be null!";
    std::shared_ptr<fs::path> test_assets_path_ptr = search_paths_vec_ptr->at(1);
    ASSERT_NE(test_assets_path_ptr, nullptr) << "test_assets path should not be null!";
    std::shared_ptr<fs::path> data_path_ptr = search_paths_vec_ptr->at(2);
    ASSERT_NE(data_path_ptr, nullptr) << "data path should not be null!";

    bool const exists = fs::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyObject *py_result = GetClassFromPython(build_path_ptr->generic_string(), test_assets_path_ptr->generic_wstring()
                                             , data_path_ptr->generic_string(), "python_tests", "append_no_args");
    ASSERT_NE(py_result, nullptr) << "Python result is null";

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_list_of_boost_filesystem_paths) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::shared_ptr<fs::path> build_path_ptr = search_paths_vec_ptr->at(0);
    ASSERT_NE(build_path_ptr, nullptr) << "Test build path should not be null!";
    std::shared_ptr<fs::path> test_assets_path_ptr = search_paths_vec_ptr->at(1);
    ASSERT_NE(test_assets_path_ptr, nullptr) << "test_assets path should not be null!";
    std::shared_ptr<fs::path> data_path_ptr = search_paths_vec_ptr->at(2);
    ASSERT_NE(data_path_ptr, nullptr) << "data path should not be null!";

    bool const exists = fs::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    std::list<fs::path> search_path_list{*test_assets_path_ptr, *build_path_ptr, *data_path_ptr};

    PyObject *py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject *py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_list_of_strings) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<std::string> search_path_strings{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_strings.emplace_back(search_path_ptr->generic_string());
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_strings);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject *py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_list_of_wstrings) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<std::wstring> search_path_strings{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_strings.emplace_back(search_path_ptr->generic_wstring());
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_strings);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject *py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, GetMethodFromPython_with_individual_paths) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::shared_ptr<fs::path> build_path_ptr = search_paths_vec_ptr->at(0);
    ASSERT_NE(build_path_ptr, nullptr) << "Test build path should not be null!";
    std::shared_ptr<fs::path> test_assets_path_ptr = search_paths_vec_ptr->at(1);
    ASSERT_NE(test_assets_path_ptr, nullptr) << "test_assets path should not be null!";
    std::shared_ptr<fs::path> data_path_ptr = search_paths_vec_ptr->at(2);
    ASSERT_NE(data_path_ptr, nullptr) << "data path should not be null!";

    bool const exists = fs::exists("python_tests.py");
    ASSERT_TRUE(exists) << "python_tests.py exists locally: " << exists << '\n';

    PyObject *py_method = GetMethodFromPython(build_path_ptr->generic_string(), test_assets_path_ptr->generic_wstring()
                                              , data_path_ptr->generic_string(), "python_tests", "append_no_args");
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    PyObject *py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, PrepareStringArgsForPython) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    const std::list<std::string> arg_string_list{"Hello", " World\n"};
    PyObject *py_args = PrepareStringArgsForPython(arg_string_list);
    ASSERT_NE(py_args, nullptr) << "PrepareStringArgsForPython returned a null pointer" << '\n';

    PyObject *py_result = CallPythonFnPyArgs(py_method, py_args);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnPyArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, PrepareWStringArgsForPython) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer evaluation result." << '\n';

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject *py_args = PrepareWStringArgsForPython(arg_wstring_list);
    ASSERT_NE(py_args, nullptr) << "PrepareWStringArgsForPython returned a null pointer" << '\n';

    PyObject *py_result = CallPythonFnPyArgs(py_method, py_args);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnPyArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnNullArgs_with_module_name_fn_name_and_search_paths) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_result = CallPythonFnNullArgs("python_tests", "append_no_args", search_path_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, CallPythonFnNullArgs_with_fn_pointer) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append_no_args", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    PyObject *py_result = CallPythonFnNullArgs(py_method);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnNullArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldLowercase);
}

TEST_F(PythonTests, CallPythonFnStringArgs_with_module_name_fn_name_and_search_paths) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    const std::list<std::string> arg_string_list{"Hello", " World\n"};
    PyObject *py_result = CallPythonFnStringArgs("python_tests", "append", search_path_list, arg_string_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnStringArgs_with_fn_pointer) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    const std::list<std::string> arg_string_list{"Hello", " World\n"};
    PyObject *py_result = CallPythonFnStringArgs(py_method, arg_string_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnWstringArgs_with_module_name_fn_name_and_search_paths) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject *py_result = CallPythonFnWStringArgs("python_tests", "append", search_path_list, arg_wstring_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnWStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnWstringArgs_with_fn_pointer) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject *py_result = CallPythonFnWStringArgs(py_method, arg_wstring_list);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnWStringArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

TEST_F(PythonTests, CallPythonFnPyArgs) {
    std::shared_ptr<std::vector<std::shared_ptr<fs::path> > > search_paths_vec_ptr = GetTestPaths();
    ASSERT_NE(search_paths_vec_ptr, nullptr) << "Test search paths should not be null!";
    ASSERT_GE(search_paths_vec_ptr->size(), 3) << "At least 3 search paths should be added to the list!";
    std::list<fs::path> search_path_list{};
    for (auto search_path_ptr : *search_paths_vec_ptr) {
        ASSERT_NE(search_path_ptr, nullptr) << "search_path should not be null!";
        search_path_list.emplace_back(*search_path_ptr);
    }

    PyObject *py_method = GetMethodFromPython("python_tests", "append", search_path_list);
    ASSERT_NE(py_method, nullptr) << "GetMethodFromPython returned a null pointer" << '\n';

    const std::list<std::wstring> arg_wstring_list{L"Hello", L" World\n"};
    PyObject *py_args = PrepareWStringArgsForPython(arg_wstring_list);
    ASSERT_NE(py_args, nullptr) << "PrepareWStringArgsForPython returned a null pointer" << '\n';

    PyObject *py_result = CallPythonFnPyArgs(py_method, py_args);
    ASSERT_NE(py_result, nullptr) << "CallPythonFnPyArgs returned a null pointer" << '\n';

    std::string const result = PyUnicode_AsUTF8(py_result);

    EXPECT_EQ(result, kHelloWorldPascalCase);
}

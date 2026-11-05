/*
 * init.cpp
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


#define PY_SSIZE_T_CLEAN

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <boost/version.hpp>
#if defined (_MSC_VER) && _MSC_VER <= 1200
#define Vector Vactor
#endif
#include <boost/python.hpp>
#include <boost/python/converter/from_python.hpp>
#include <boost/filesystem/path.hpp>
#if defined (_MSC_VER) && _MSC_VER <= 1200
#undef Vector
#endif

#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include <compile.h>
#if ((PY_VERSION_HEX) < 0x030B0000)
#include <eval.h>
#endif
#include <stdio.h>
#include <math.h>

#include "root_generic/configxml.h"
#include "root_generic/vs_globals.h"
#include "vegadisk/vsfilesystem.h"
#include "src/vs_logging.h"
#include "src/python/init.h"
#include "src/python/python_compile.h"
#include "src/python/python_class.h"
#include "cmd/unit_generic.h"
#include "src/python/config/python_utils.h"
#include "cmd/vega_py_run.h"
#include "cmd/basecomputer.h"

#if defined (_WIN32) && !defined (__CYGWIN__)
#include <direct.h>
#endif
class Unit;
//FROM_PYTHON_SMART_POINTER(Unit)


#ifdef _WIN32
//Python 2.5 doesn't seem to like forward-slashes.
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace fs = boost::filesystem;

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
        VS_LOG_FLUSH_EXIT(fatal, "Unable to locate Python!", -1);
    }

    return return_value;
}
}
#endif

void Python::initpaths() {
    /*
     *  char pwd[2048];
     *  getcwd (pwd,2047);
     *  pwd[2047]='\0';
     *  for (int i=0;pwd[i]!='\0';i++) {
     *       if (pwd[i]=='\\')
     *               pwd[i]=DELIM;
     *  }
     */
    std::string moduledir(configuration().data.python_modules);
    std::string basesdir(configuration().data.python_bases);

    /*
     *  std::string changepath ("import sys\nprint sys.path\nsys.path = ["
     *                       "\""+std::string(pwd)+DELIMSTR"modules"DELIMSTR"builtin\""
     *                       ",\""+std::string(pwd)+DELIMSTR+moduledir+string("\"")+
     *                       ",\""+std::string(pwd)+DELIMSTR+basesdir + string("\"")+
     *                       "]\n");
     */
    std::string modpaths("\"\",");
    //Find all the mods dir (ignore homedir)
    for (unsigned int i = 1; i < VSFileSystem::Rootdir.size(); i++) {
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + moduledir + PATHSEP "builtin\",";
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + moduledir + PATHSEP "quests\",";
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + moduledir + PATHSEP "missions\",";
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + moduledir + PATHSEP "ai\",";
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + moduledir + "\",";
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + basesdir + "\",";
        modpaths += "r\"" + VSFileSystem::Rootdir[i] + PATHSEP + "python" + PATHSEP + "base_computer\"";
        if (i + 1 < VSFileSystem::Rootdir.size()) {
            modpaths += ",";
        }
    }
    /*
     *  string::size_type backslash;
     *  while ((backslash=modpaths.find("\\"))!=std::string::npos) {
     *  modpaths[backslash]='/';
     *  }*/
    std::string changepath("import sys\nprint(sys.path)\nsys.path = [" + modpaths + "] + sys.path\n");
    /*
     *  std::string changepath ("import sys\nprint sys.path\nsys.path = ["
     *                       "\""+VSFileSystem::datadir+DELIMSTR"modules"DELIMSTR"builtin\""
     *                       ",\""+VSFileSystem::datadir+DELIMSTR+moduledir+string("\"")+
     *                       ",\""+VSFileSystem::datadir+DELIMSTR+basesdir + string("\"")+
     *                       "]\n");
     */
    VegaPyRunString(changepath);
}

void Python::reseterrors() {
    if (PyErr_Occurred()) {
        VS_LOG_AND_FLUSH(error, "void Python::reseterrors(): Python error occurred");
        PyErr_Print();
        PyErr_Clear();
        VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    }
}

#if BOOST_VERSION != 102800

static void *Vector_convertible(PyObject * p) {
    return PyTuple_Check(p) ? p : 0;
}

static void Vector_construct(PyObject * source, boost::python::converter::rvalue_from_python_stage1_data * data) {
    void *const storage = ((boost::python::converter::rvalue_from_python_storage<Vector> *) data)->storage.bytes;
    new(storage) Vector(0, 0, 0);
    //Fill in QVector values from source tuple here
    //details left to reader.
    Vector *vec = (Vector *) storage;
    static char fff[4] = "fff"; //added by chuck_starchaser, to kill a warning
    PyArg_ParseTuple(source, fff, &vec->i, &vec->j, &vec->k);
    data->convertible = storage;
}

static void QVector_construct(PyObject * source, boost::python::converter::rvalue_from_python_stage1_data * data) {
    void *const storage = ((boost::python::converter::rvalue_from_python_storage<QVector> *) data)->storage.bytes;
    new(storage) QVector(0, 0, 0);
    //Fill in QVector values from source tuple here
    //details left to reader.
    QVector *vec = (QVector *) storage;
    static char ddd[4] = "ddd"; //added by chuck_starchaser, to kill a warning
    PyArg_ParseTuple(source, ddd, &vec->i, &vec->j, &vec->k);
    data->convertible = storage;
}

#endif

void Python::init() {
    static bool isinit = false;
    if (isinit) {
        return;
    }
    isinit = true;

    bool was_already_initialized = Py_IsInitialized() != 0;
    if (was_already_initialized) {
        return;
    }

    // initialize python library
    PyPreConfig py_pre_config;
    PyPreConfig_InitPythonConfig(&py_pre_config);
    py_pre_config.coerce_c_locale = 1;

    PyStatus status = Py_PreInitialize(&py_pre_config);
    if (PyStatus_Exception(status) != 0) {
        VS_LOG_AND_FLUSH(fatal, "Python::init(): PyPreInitialize failed");
        PyErr_Print();
        VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
        Py_ExitStatusException(status);
    }

    // These functions add these modules to the builtin package
    InitVS();
    InitBriefing();
    InitBase();
    InitDirector();
    InitBaseComputer();

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
        VS_LOG_AND_FLUSH(fatal, "Python::init(): PyConfig_SetString failed");
        VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
        Py_ExitStatusException(status);
    }
#endif

    // Now we can do python things about them and initialize them
    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);
    if (PyStatus_Exception(status) != 0) {
        VS_LOG_AND_FLUSH(fatal, "Python::init(): Py_InitializeFromConfig failed");
        PyErr_Print();
        VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
        Py_ExitStatusException(status);
    }

    boost::python::import("base_computer"); // REQUIRED
    initpaths();

#if BOOST_VERSION != 102800
    boost::python::converter::registry::insert(Vector_convertible,
            QVector_construct,
            boost::python::type_id<QVector>());
    boost::python::converter::registry::insert(Vector_convertible, Vector_construct, boost::python::type_id<Vector>());
#endif
#if (PY_VERSION_HEX < 0x03000000)
    InitBriefing2();
    InitVS2();
#endif
    VS_LOG(important_info, "testing Python integration");
    std::string python_snippet_to_run_1("import sys\nprint(sys.path)\n");
    VegaPyRunString(python_snippet_to_run_1);

    const std::string python_snippet_to_run_2{
            "import VS\n"
            "print("
            "\"Engine Version: {0} \\nAsset API Version: {1}\".format("
            "'.'.join([str(i) for i in VS.EngineVersion().GetVersion()]),"
            "VS.EngineVersion().GetAssetAPIVersion()"
            ")"
            ")"
    };
    VegaPyRunString(python_snippet_to_run_2);
#if (PY_VERSION_HEX < 0x03000000)
    InitDirector2();
    InitBase2();
#endif
}

void Python::test() {
    /* initialize vegastrike module so that
     *  'import VS' works in python */

    /* There should be a python script automatically executed right after
     *  initVegastrike to rebind some of the objects into a better hierarchy,
     *  and to install wrappers for class methods, etc.
     *
     *  This script should look something like <<EOF
     *  import VS
     *  import sys
     *
     * #Set up output redirection
     *  sys.stdout = VS.IO()
     *  sys.stderr = sys.stdout
     *
     * #Make Var look like a nested 'class'
     *  VS.Config.Var = VS.Var()
     *
     *  EOF
     *  This should be executed with PyRun_SimpleString(char *command)
     *  See defs.py for some recipes that should go in there
     *
     *  Other useful Python functions:
     *
     *  PyObject* Py_CompileString(char *str, char *filename, int start)
     *  Return value: New reference.
     *  Parse and compile the Python source code in str, returning the resulting code object. The start token is given by start; this can be used to constrain the code which can be compiled and should be Py_eval_input, Py_file_input, or Py_single_input. The filename specified by filename is used to construct the code object and may appear in tracebacks or SyntaxError exception messages. This returns NULL if the code cannot be parsed or compiled.
     *
     *  This would be the preferred mode of operation for AI scripts
     */

//char buffer[128];
//PythonIOString::buffer << endl << '\0';
//vs_config->setVariable("data","test","NULL");
//VSFileSystem::vs_fprintf(stdout, "%s", vs_config->getVariable("data","test", string()).c_str());
//VSFileSystem::vs_fprintf(stdout, "output %s\n", PythonIOString::buffer.str());
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogs();
}

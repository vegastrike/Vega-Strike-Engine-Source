/*
 * python_compile.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "cmd/unit_generic.h"
#include "python_compile.h"
#include <compile.h>
#if ((PY_VERSION_HEX) < 0x030B0000)
#include <eval.h>
#endif
#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "init.h"
#include "universe_util.h"
#include "in_kb_data.h"
#include "vs_logging.h"

Hashtable<string, PyObject, 1023> compiled_python;

char *LoadString(const char *filename) {
    FILE *fp = VSFileSystem::vs_open(filename, "r");
    if (!fp) {
        return NULL;
    }
    VSFileSystem::vs_fseek(fp, 0, SEEK_END);
    long len = VSFileSystem::vs_ftell(fp);
    char *retval = NULL;
    VSFileSystem::vs_fseek(fp, 0, SEEK_SET);
    if (len) {
        retval = (char *) malloc((len + 2) * sizeof(char));
        len = VSFileSystem::vs_read(retval, 1, len, fp);
        retval[len] = '\0';
    }
    VSFileSystem::vs_close(fp);
    return retval;
}

std::string getCompilingName(const std::string &name) {
    std::string compiling_name = VSFileSystem::homedir + DELIMSTR + name;
    return compiling_name;
}

void InterpretPython(const std::string &name) {
    char *temp = strdup(getCompilingName(name).c_str());
    FILE *fp = VSFileSystem::vs_open(name.c_str(), "r");
    if (fp) {
        PyRun_SimpleFile(fp, temp);
        Python::reseterrors();
        VSFileSystem::vs_close(fp);
    }
    free(temp);
}

PyObject *CompilePython(const std::string &name) {
    Python::reseterrors();
    PyObject * retval = compiled_python.Get(name);
    Python::reseterrors();
    if (retval) {
        return retval;
    }
    char *str = LoadString(name.c_str());
    if (str) {
        VS_LOG(info, (boost::format("Compiling python module %1$s\n") % name));

        std::string compiling_name = getCompilingName(name).c_str();
        char *temp = strdup(compiling_name.c_str());

        retval = Py_CompileString(str, temp, Py_file_input);
        if (retval) {
            compiled_python.Put(name, retval);
        }
        free(temp);
        free(str);
    }
    return retval;
}

extern PyObject *PyInit_VS;

void CompileRunPython(const std::string &filename) {
#if (PY_VERSION_HEX >= 0x030B0000)
    Python::reseterrors();
    InterpretPython(filename);
    Python::reseterrors();
#else
    static bool ndebug_libs = XMLSupport::parse_bool(vs_config->getVariable("AI", "compile_python", "true"));
    if (ndebug_libs) {
        Python::reseterrors();
        PyObject * CompiledProgram = CompilePython(filename);
        Python::reseterrors();
        if (CompiledProgram) {
            PyObject * m, *d;
            static char main_str[16] = "__main__"; //by chuck_starchaser, to squash a warning
            if ((m = PyImport_AddModule(main_str)) != NULL) {
                PyObject * localdict = PyDict_New();
                if ((d = PyModule_GetDict(m)) != NULL) {
                    PyObject * exe = PyEval_EvalCode(
#if (PY_VERSION_HEX >= 0x03020000)
                            CompiledProgram,
#else
                            (PyCodeObject*)CompiledProgram,
#endif
                            d, localdict);
                    Py_XDECREF(exe);
                    //unref exe?
                }
                Py_XDECREF(localdict);
            }
        }
    } else {
        Python::reseterrors();
        InterpretPython(filename);
        Python::reseterrors();
    }
#endif
}

PyObject *CreateTuple(const std::vector<PythonBasicType> &values) {
    PyObject * retval = PyTuple_New(values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
        PyObject * val = values[i].NewObject();
        if (val)
            PyTuple_SET_ITEM(retval, i, val);
    }
    return retval;
}

static void pySetScratchVector(const KBSTATE k) {
    switch (k) {
        case PRESS:
            UniverseUtil::setScratchVector(Vector(1, 1, 0));
            break;
        case RELEASE:
            UniverseUtil::setScratchVector(Vector(0, 1, 0));
            break;
        case UP:
            UniverseUtil::setScratchVector(Vector(0, 0, 0));
            break;
        case DOWN:
            UniverseUtil::setScratchVector(Vector(1, 0, 0));
            break;
        default:
            break;
    }
}

void RunPythonPress(const KBData &s, KBSTATE k) {
    if (k == PRESS && s.data.length()) {
        pySetScratchVector(k);
        CompileRunPython(s.data);
        UniverseUtil::setScratchVector(Vector(0, 0, 0));
    }
}

void RunPythonRelease(const KBData &s, KBSTATE k) {
    if (k == RELEASE && s.data.length()) {
        pySetScratchVector(k);
        CompileRunPython(s.data);
        UniverseUtil::setScratchVector(Vector(0, 0, 0));
    }
}

void RunPythonToggle(const KBData &s, KBSTATE k) {
    if ((k == RELEASE || k == PRESS) && s.data.length()) {
        pySetScratchVector(k);
        CompileRunPython(s.data);
        UniverseUtil::setScratchVector(Vector(0, 0, 0));
    }
}

void RunPythonPhysicsFrame(const KBData &s, KBSTATE k) {
    if ((k == DOWN || k == UP) && s.data.length()) {
        pySetScratchVector(k);
        CompileRunPython(s.data);
        UniverseUtil::setScratchVector(Vector(0, 0, 0));
    }
}


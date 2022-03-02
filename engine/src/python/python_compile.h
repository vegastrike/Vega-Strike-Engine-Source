/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _PYTHON_COMPILE_H_
#define _PYTHON_COMPILE_H_

//Resets variable for use by python config.h
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#include <Python.h>
#include "hashtable.h"
#include <string>
#include <compile.h>

extern Hashtable<std::string, PyObject, 1023> compiled_python;

class PythonBasicType {
public:
    std::string objects;
    long objecti;
    double objectd;
    enum { MYSTRING, MYLONG, MYDOUBLE } type;

    PythonBasicType(const std::string &mystr) {
        type = MYSTRING;
        objects = mystr;
    }

    PythonBasicType(const long mystr) {
        type = MYLONG;
        objecti = mystr;
    }

    PythonBasicType(const float mystr) {
        type = MYDOUBLE;
        objectd = mystr;
    }

    PyObject *NewObject() const {
        switch (type) {
            case MYSTRING:
                return PyUnicode_FromString(objects.c_str());

            case MYLONG:
                return PyLong_FromLong(objecti);

            case MYDOUBLE:
                return PyFloat_FromDouble(objectd);
        }
        return NULL;
    }
};

void InterpretPython(const std::string &filename);
PyObject *CompilePython(const std::string &filename);
void CompileRunPython(const std::string &filename);
PyObject *CreateTuple(const std::vector<PythonBasicType> &values);

///Warning: The basic pointer class does NOTHING for the user.
///NO Refcounts...if python holds onto this for longer than it can...
///CRASH!!
template<class T>
class BasicPointer {
    T *myitem;
public:
    BasicPointer(T *myitem) {
        this->myitem = myitem;
    }

    T &operator*() {
        return *myitem;
    }
};
#endif


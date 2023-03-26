/*
 * pythonai.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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
#include <Python.h> // TODO: Remove direct inclusion of this file?
#include "fire.h"
#include <assert.h>

using Orders::FireAt;
class PythonAI : public FireAt {
    PyObject *self;

    PythonAI(const PythonAI &a) : FireAt(a) {
        assert(0);
    }

    static PythonAI *last_ai;
protected:
    virtual void Destruct();
public:
    PythonAI(PyObject *self, float reaction_time, float agressivity);
    virtual void Execute();
    static void default_Execute(FireAt &pay);
    static void InitModuleAI();
    static PythonAI *LastAI();
    static PythonAI *Factory(const std::string &file);
    virtual ~PythonAI();
};


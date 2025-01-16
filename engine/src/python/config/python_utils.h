/*
 * python_utils.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGA_STRIKE_ENGINE_RESOURCE_PYTHON_UTILS_H
#define VEGA_STRIKE_ENGINE_RESOURCE_PYTHON_UTILS_H

#include <string>

// See https://stackoverflow.com/q/804894/5276890
struct _object;
typedef _object PyObject;

std::string GetPythonPath();

// A utility function to call a function and get a PyObject as a result
PyObject* GetClassFromPython(
    std::string build_path,
    std::wstring path_string,
    std::string module_name,
    std::string function_name);


#endif //VEGA_STRIKE_ENGINE_RESOURCE_PYTHON_UTILS_H
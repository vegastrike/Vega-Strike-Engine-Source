#ifndef UNITJSONFACTORY_H
#define UNITJSONFACTORY_H

/*
 * unit_json_factory.cpp
 *
 * Copyright (C) 2021 Roy Falk
 * Copyright (C) 2022 Stephen G. Tuggy
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

#include <string>

#include "vsfilesystem.h"

class UnitJSONFactory {
    static std::string DEFAULT_ERROR_VALUE;

public:
    static void ParseJSON(VSFileSystem::VSFile &file);
};
#endif // UNITJSONFACTORY_H

/*
 * unit_optimize_factory.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy, and
 * other Vega Strike contributors
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_OPTIMIZE_FACTORY_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_OPTIMIZE_FACTORY_H

#include <map>
#include <string>

#include "vsfilesystem.h"

class UnitOptimizeFactory
{
public:
    static void ParseJSON(VSFileSystem::VSFile &file);
    static void RecursiveParse(std::map<std::string, std::string> unit_attributes,
                       const std::string& json_text, bool is_root);
};

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_OPTIMIZE_FACTORY_H

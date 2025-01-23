/*
 * engineVersion.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
#ifndef VEGA_STRIKE_ENGINE_CMD_VERSION_H
#define VEGA_STRIKE_ENGINE_CMD_VERSION_H

#include <string>
#include <stdint.h>

#include "boost/tuple/tuple.hpp"

namespace VegaStrike {

//typedef ::boost::tuple<uint8_t, uint8_t, uint8_t, std::wstring> EngineVersionTuple;

class EngineVersionData {
public:
    EngineVersionData();
    ~EngineVersionData();

    // Game Engine Version <major>.<minor>.<patch>.<hash>
    // presented as a tuple
    boost::python::tuple GetVersion() const;

    // Asset API Version, simply numeric incrementer
    uint16_t GetAssetAPIVersion() const;
protected:
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    std::wstring hash;

    uint16_t assetApiVersion;
};
}

#endif //VEGA_STRIKE_ENGINE_CMD_VERSION_H

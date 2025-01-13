/*
 * game_config.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_CONFIG_GAME_CONFIG_H
#define VEGA_STRIKE_ENGINE_CONFIG_GAME_CONFIG_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <set>
#include <map>
#include <exception>
#include <iostream>
#include <cstdint>
#include <type_traits>

#include "vs_logging.h"

namespace pt = boost::property_tree;

namespace vega_config {

class GameConfig {
private:
    // This is probably unique enough to ensure no collision
//    constexpr static const char DEFAULT_ERROR_VALUE[] {"vega_config::GetGameConfig().GetVar DEFAULT_ERROR_VALUE"};

    static std::string EscapedString(std::string const & input);
    static boost::shared_ptr<pt::iptree> variables_();

public:

    void LoadGameConfig(const std::string &filename);

    template<typename T>
    T GetVariable(std::string const & path, T default_value) {
        return variables_()->get(path, default_value);
    }

    inline std::string GetEscapedString(std::string const & path, std::string const & default_value) {
        return EscapedString(variables_()->get(path, default_value));
    }

    inline std::string GetString(std::string const & path, std::string const & default_value) {
        return variables_()->get(path, default_value);
    }

    inline float GetFloat(std::string const & path, float default_value) {
        return variables_()->get(path, default_value);
    }

    inline double GetDouble(std::string const & path, double default_value) {
        return variables_()->get(path, default_value);
    }

    inline size_t GetSizeT(std::string const & path, uintmax_t default_value) {
        return variables_()->get(path, static_cast<size_t>(default_value));
    }

    inline uint8_t GetUInt8(std::string const & path, uint8_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline uint16_t GetUInt16(std::string const & path, uint16_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline uint32_t GetUInt32(std::string const & path, uint32_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline uint64_t GetUInt64(std::string const & path, uint64_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline uintmax_t GetUIntMaxT(std::string const & path, uintmax_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline int8_t GetInt8(std::string const & path, int8_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline int16_t GetInt16(std::string const & path, int16_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline int32_t GetInt32(std::string const & path, int32_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline int64_t GetInt64(std::string const & path, int64_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline intmax_t GetIntMaxT(std::string const & path, intmax_t default_value) {
        return variables_()->get(path, default_value);
    }

    inline bool GetBool(std::string const & path, bool default_value) {
        return variables_()->get(path, default_value);
    }
};

extern GameConfig& GetGameConfig();

}

#endif //VEGA_STRIKE_ENGINE_CONFIG_GAME_CONFIG_H

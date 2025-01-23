/*
 * abstract_config.h
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
#ifndef VEGA_STRIKE_ENGINE_ABSTRACT_CONFIG_H
#define VEGA_STRIKE_ENGINE_ABSTRACT_CONFIG_H

#include <string>

class AbstractConfig {
private:
    static std::string DEFAULT_ERROR_VALUE;
    virtual inline std::string _GetStringVariable(std::string const &section, std::string const &name) = 0;

    //template <class T>
    //inline T GetVariable(std::string const &result, std::string const &default_value);

public:
    AbstractConfig();
};

#endif //VEGA_STRIKE_ENGINE_ABSTRACT_CONFIG_H

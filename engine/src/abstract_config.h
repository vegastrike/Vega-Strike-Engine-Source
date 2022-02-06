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

#ifndef ABSTRACTCONFIG_H
#define ABSTRACTCONFIG_H

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

#endif // ABSTRACTCONFIG_H

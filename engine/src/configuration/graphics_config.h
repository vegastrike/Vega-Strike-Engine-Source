/*
 * graphics_config.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

#ifndef VEGA_STRIKE_ENGINE_PYTHON_CONFIG_GRAPHICS_CONFIG_H
#define VEGA_STRIKE_ENGINE_PYTHON_CONFIG_GRAPHICS_CONFIG_H

#include <boost/json.hpp>

// TODO: remove the other GraphicsConfig and rename this
struct Graphics2Config {
    int screen{0};
    int resolution_x{1920};
    int resolution_y{1080};

    Graphics2Config() = default;
    Graphics2Config(boost::json::object object);
};



#endif // VEGA_STRIKE_ENGINE_PYTHON_CONFIG_GRAPHICS_CONFIG_H
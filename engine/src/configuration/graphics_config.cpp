/*
 * graphics_config.cpp
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
#include <string>
#include <iostream>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>

#include "graphics_config.h"
#include "resource/json_utils.h"


// TODO: delete this. Graphics2Config should be generated automatically from config.json
Graphics2Config::Graphics2Config(boost::json::object object) {
    if (object.if_contains("graphics")) {
        boost::json::object graphics = object.at("graphics").get_object();
        ConditionalJsonGet(graphics, screen, "screen");
        ConditionalJsonGet(graphics, resolution_x, "resolution_x");
        ConditionalJsonGet(graphics, resolution_y, "resolution_y");
    }
}





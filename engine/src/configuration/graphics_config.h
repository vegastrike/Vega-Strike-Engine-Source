/*
 * graphics_config.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2002-2019 pyramid3d and other Vega Strike contributors
 * Copyright (C) 2019-2024 Stephen G. Tuggy, Benjamen R. Meyer,
 *                        Roy Falk, and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGA_STRIKE_ENGINE_PYTHON_CONFIG_GRAPHICS_CONFIG_H
#define VEGA_STRIKE_ENGINE_PYTHON_CONFIG_GRAPHICS_CONFIG_H

// Core JSON-based utility header
#include "json.h"

// External library references
#include <boost/json.hpp>
#include <string>

/**
 * @brief Graphics2Config is a JSON-backed configuration structure
 *        for describing basic graphics settings.
 *
 * This struct currently demonstrates:
 *  - In-engine usage of screen indexing (e.g. which monitor to use).
 *  - Output resolution for x and y dimensions, defaulting to 1920x1080.
 *
 * The constructor can parse from a boost::json::object, enabling flexible
 * assignment from JSON-serialized configuration data.
 */
struct Graphics2Config
{
    /// Zero-based index for which screen or display to use.
    int screen {0};

    /// Resolution width and height, default is 1920x1080.
    int resolution_x {1920};
    int resolution_y {1080};

    /// Default constructor sets fields to typical 1080p display.
    Graphics2Config() = default;

    /**
     * @brief Construct from a boost::json::object
     *        e.g. from reading user config "graphics" section.
     *
     * @param object A boost::json::object presumably containing the keys
     *               "screen", "resolution_x", and "resolution_y".
     */
    explicit Graphics2Config(const boost::json::object &object)
    {
        // Retrieve integer values safely, falling back to existing defaults
        // if the key is absent or not an integer type.
        if (object.if_contains("screen")) {
            const auto &val = object.at("screen");
            if (val.is_int64()) {
                screen = static_cast<int>(val.as_int64());
            }
        }
        if (object.if_contains("resolution_x")) {
            const auto &val = object.at("resolution_x");
            if (val.is_int64()) {
                resolution_x = static_cast<int>(val.as_int64());
            }
        }
        if (object.if_contains("resolution_y")) {
            const auto &val = object.at("resolution_y");
            if (val.is_int64()) {
                resolution_y = static_cast<int>(val.as_int64());
            }
        }
    }
};

#endif // VEGA_STRIKE_ENGINE_PYTHON_CONFIG_GRAPHICS_CONFIG_H
